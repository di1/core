/// <reference path="IChart.ts" />
/// <reference path="ILinearEquation.ts" />

class CandleChart {

  private symbol: string;
  private conn: WebSocket;
  private chart_canvas: HTMLCanvasElement | null;
 
  private NUM_TICKS: number = 20;
  private PADDING_BOT: number = 15;
  private PADDING_TOP: number = 15;
  private CANDLE_WIDTH: number = 10;


  private ROOT_CHART: RootChartObject | undefined = undefined;
  private CANDLE_UPDATES_SENT: number = 0;
  private CANDLE_SPACING: number = 5;

  private MOUSE_X: number = 0;
  private MOUSE_Y: number = 0;

  constructor(symbol: string) {
    this.chart_canvas = <HTMLCanvasElement> document.getElementById("chart");

    if (!this.chart_canvas) {
      console.error("chart does not exist in html");
    }

    this.symbol = symbol;
    this.conn = new WebSocket("ws://localhost:7681", "lws-minimal");
    this.conn.onopen = this.onOpen.bind(this);
    this.conn.onclose = this.onClose.bind(this);
    this.conn.onmessage = this.onMessage.bind(this);
  
    //this.rescaleCanvas(this.chart_canvas);

    this.chart_canvas.onwheel = this.onMouseWheelEvent.bind(this);
    this.chart_canvas.onmousemove = this.onMouseMove.bind(this);
  }

  private onMessage(evt: MessageEvent) {

    var genericMsg = JSON.parse(evt.data);

    if (genericMsg['chart']) {

      let latestChart: RootChartObject | undefined = genericMsg;

      if (!latestChart) {
        console.error('onMessage latestChart == undefined');
        return;
      }

      this.ROOT_CHART = latestChart;
      this.drawFull(this.ROOT_CHART);

    } else if (genericMsg['latest_candle']) {

      let updateCandle: LatestChartCandle | undefined = genericMsg;

      if (!updateCandle) {
        console.error('onMessage updateCandle == undefined');
        return;
      }
      if (!this.ROOT_CHART) {
        console.error('onMessage updateCandle happened before init');
        return;
      }

      if (this.ROOT_CHART.chart[this.ROOT_CHART.chart.length-1].candle.s ==
          updateCandle.latest_candle.candle.s) {

        this.ROOT_CHART.chart[this.ROOT_CHART.chart.length-1].candle =
          updateCandle.latest_candle.candle;
        this.drawFull(this.ROOT_CHART);
      } else {
        this.ROOT_CHART.chart.push({candle: updateCandle.latest_candle.candle});
        this.drawFull(this.ROOT_CHART);
      }
    } else {
      console.error('invalid response from server');
    }
  }

  private onOpen(evt: Event) {
    this.conn.send('init|' + this.symbol);
  }

  private onClose() {
    console.log('websocket connection closed');
  }

  public rescaleCanvas(canvas: any): void {
    // finally query the various pixel ratios
    let ctx = canvas.getContext('2d');

    let devicePixelRatio = window.devicePixelRatio || 1;

    let backingStoreRatio = ctx.webkitBackingStorePixelRatio ||
                        ctx.mozBackingStorePixelRatio ||
                        ctx.msBackingStorePixelRatio ||
                        ctx.oBackingStorePixelRatio ||
                        ctx.backingStorePixelRatio || 1;
    let ratio = devicePixelRatio / backingStoreRatio;

    console.log(devicePixelRatio);

    // upscale the canvas if the two ratios don't match
    if (devicePixelRatio !== backingStoreRatio) {

      let oldWidth = canvas.width;
      let oldHeight = canvas.height;

      canvas.width = oldWidth * ratio;
      canvas.height = oldHeight * ratio;

      canvas.style.width = oldWidth + 'px';
      canvas.style.height = oldHeight + 'px';

      // now scale the context to counter
      // the fact that we've manually scaled
      // our canvas element
      ctx.scale(ratio, ratio);
    }
  }

  private onMouseMove(evt: MouseEvent) {
    if (!this.chart_canvas)
      return;
    const rect = this.chart_canvas.getBoundingClientRect();
    const x = evt.clientX;
    const y = evt.clientY;

    let scaleX: number = this.chart_canvas.width / rect.width;    // relationship bitmap vs. element for X
    let scaleY: number = this.chart_canvas.height / rect.height;  // relationship bitmap vs. element for Y

    this.MOUSE_X = (x - rect.left) * scaleX;
    this.MOUSE_Y = (y - rect.top) * scaleY;
  }

  private getChartRange(candles: Chart[], start_index: number): ChartRange {
    let gmax: number = Number.MIN_VALUE;
    let gmin: number = Number.MAX_VALUE;

    for (let i: number = start_index; i < candles.length; ++i) {
      let lmax: number = Math.max(candles[i].candle.o,candles[i].candle.h,
                    candles[i].candle.l, candles[i].candle.c);
      let lmin: number = Math.min(candles[i].candle.o,candles[i].candle.h,
                    candles[i].candle.l, candles[i].candle.c);

      if (lmax > gmax)
        gmax = lmax;
      if (lmin < gmin)
        gmin = lmin;

    }

    let r: ChartRange = {max: gmax, min: gmin};
    return r;
  }

  private getPriceWidth(ctx: CanvasRenderingContext2D): number {
    return ctx.measureText(' 0000.0000').width;
  }

  private onMouseWheelEvent(evt: WheelEvent) {
    console.log(evt.deltaY);
    if (evt.deltaY > 0)
      this.CANDLE_WIDTH += 1;
    if (evt.deltaY < 0)
      this.CANDLE_WIDTH -= 1;
  }

  private drawFull(chart: RootChartObject) {
    if (!this.chart_canvas) {
      console.error('unable to get canvas element');
      return;
    }

    let ctx: CanvasRenderingContext2D | null = 
      this.chart_canvas.getContext('2d');

    if (!ctx) {
      console.error('contex == undefined');
      return;
    }

    ctx.canvas.width = window.innerWidth;
    ctx.canvas.height = window.innerHeight;

    ctx.font = "normal 1.4em Monospace";

    let drawing_width: number = this.chart_canvas.width;
    let drawing_height: number = 
      this.chart_canvas.height-this.PADDING_BOT;

    ctx.strokeStyle = 'white';
    ctx.textBaseline = 'middle';

    ctx.fillStyle = '#131722'; 
    ctx.fillRect(0,0,this.chart_canvas.width, this.chart_canvas.height);
    ctx.fillStyle = 'white';

    let candles: Chart[] = chart.chart;

    let num_displayable_candles: number = 
      (this.chart_canvas.width-this.getPriceWidth(ctx))/(this.CANDLE_WIDTH+this.CANDLE_SPACING);

    num_displayable_candles -= 1;

    let start_index: number;
    if (num_displayable_candles < candles.length)
      start_index = Math.floor(candles.length-num_displayable_candles);
    else
      start_index = 0;


    let priceRange: ChartRange = this.getChartRange(candles, start_index);


    ctx.moveTo(drawing_width-this.getPriceWidth(ctx), 0);
    ctx.lineTo(drawing_width-this.getPriceWidth(ctx), this.chart_canvas.height);
    ctx.stroke();

    let pixel_to_price: LinearEquation = 
      new LinearEquation(this.PADDING_TOP, priceRange.max, drawing_height-this.PADDING_BOT, priceRange.min);

    let price_to_pixel: LinearEquation = 
      new LinearEquation(priceRange.min, drawing_height, priceRange.max,
                        this.PADDING_TOP);


    // DRAW THE PRICE TICKS
    let inc: number = (priceRange.max-priceRange.min)/this.NUM_TICKS;
    for (let i: number = priceRange.min; i <= priceRange.max; i += inc) {
      ctx.fillText("-" + (i/10000).toFixed(4),
                   drawing_width-this.getPriceWidth(ctx),
                   price_to_pixel.eval(i));

      ctx.strokeStyle = 'gray';
      ctx.setLineDash([2,5]);
      ctx.beginPath();
      ctx.moveTo(0, price_to_pixel.eval(i));
      ctx.lineTo(drawing_width-this.getPriceWidth(ctx),
                 price_to_pixel.eval(i));
      ctx.stroke();
      ctx.setLineDash([]);
    }

    // DRAW THE CANDLES
    let last_color: string = '';
    for (let i: number = start_index; i < candles.length; ++i) {
      let width_offset: number = 
        ((i-start_index)*(this.CANDLE_WIDTH+this.CANDLE_SPACING));
      
      ctx.fillStyle = 'blue';
      ctx.beginPath();
      ctx.moveTo(width_offset + this.CANDLE_WIDTH/2.0,
                 price_to_pixel.eval(candles[i].candle.h));
      ctx.lineTo(width_offset + this.CANDLE_WIDTH/2.0,
                 price_to_pixel.eval(candles[i].candle.l));
      ctx.stroke();

      if (candles[i].candle.o > candles[i].candle.c) {
        ctx.fillStyle = '#EF5350';
        last_color = ctx.fillStyle;
        ctx.fillRect(
          width_offset,
          price_to_pixel.eval(candles[i].candle.c),
          this.CANDLE_WIDTH,
          price_to_pixel.eval(candles[i].candle.o)-
            price_to_pixel.eval(candles[i].candle.c));

        ctx.fillStyle = 'black';
      } else if (candles[i].candle.o < candles[i].candle.c) {
        ctx.fillStyle = '#26A69A';
        last_color = ctx.fillStyle;

        ctx.fillRect(
          width_offset,
          price_to_pixel.eval(candles[i].candle.o),
          this.CANDLE_WIDTH,
          price_to_pixel.eval(candles[i].candle.c)-
            price_to_pixel.eval(candles[i].candle.o));
        ctx.fillStyle = 'black';
      } else {
        ctx.fillStyle = 'white';
        ctx.beginPath();
        ctx.moveTo(width_offset,
                   price_to_pixel.eval(candles[i].candle.o));
        ctx.lineTo(width_offset + this.CANDLE_WIDTH,
                   price_to_pixel.eval(candles[i].candle.c));
        ctx.stroke();
      }
    }

    
    ctx.fillStyle = last_color; 
    ctx.fillRect(drawing_width-this.getPriceWidth(ctx),
                 price_to_pixel.eval(candles[candles.length-1].candle.c) - (20.0/2.0),
                 this.getPriceWidth(ctx), 20);
    ctx.fillStyle = 'white';
    ctx.fillText("-" + ((candles[candles.length-1].candle.c)/10000.0).toFixed(4),drawing_width-this.getPriceWidth(ctx),
                 price_to_pixel.eval(candles[candles.length-1].candle.c));

    // draw the mouse cross
    ctx.fillStyle = 'white'; 
    ctx.fillRect(drawing_width-this.getPriceWidth(ctx),
                 this.MOUSE_Y - (20.0/2.0),
                 this.getPriceWidth(ctx), 20);
    ctx.fillStyle = 'black';
    ctx.fillText("-" + ((pixel_to_price.eval(this.MOUSE_Y))/10000).toFixed(4),drawing_width-this.getPriceWidth(ctx),
                 this.MOUSE_Y);

    ctx.strokeStyle = 'white';
    ctx.setLineDash([5,7]);
    ctx.beginPath();
    ctx.moveTo(0, this.MOUSE_Y);
    ctx.lineTo(drawing_width-this.getPriceWidth(ctx), this.MOUSE_Y);
    ctx.moveTo(this.MOUSE_X, 0);
    ctx.lineTo(this.MOUSE_X, drawing_height);
    ctx.stroke();
    ctx.setLineDash([]);

    this.CANDLE_UPDATES_SENT += 1;
    // sync chart every 100 updates
    if (this.CANDLE_UPDATES_SENT % 100 == 0) {
      this.CANDLE_UPDATES_SENT = 0;
      this.conn.send('init|' + this.symbol);
    } else {
      this.conn.send('latest|' + this.symbol);
    }
  }
}
