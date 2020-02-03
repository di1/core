/// <reference path="IChart.ts" />
/// <reference path="ILinearEquation.ts" />

class CandleChart {

  private symbol: string;
  private conn: WebSocket;
  private chart_canvas: HTMLCanvasElement | null;
 
  private NUM_TICKS: number = 20;
  private PADDING_BOT: number = 15;
  private PADDING_TOP: number = 15;
  private CANDLE_WIDTH: number = 15;


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
  
    this.rescaleCanvas(this.chart_canvas);

    this.chart_canvas.onwheel = this.onMouseWheelEvent.bind(this);

  }

  private onMessage(evt: MessageEvent) {
    let latestChart: RootChartObject | undefined = JSON.parse(evt.data);

    if (!latestChart) {
      console.error('onMessage latestChart == undefined');
      return;
    }

    this.drawFull(latestChart);
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

    let drawing_width: number = this.chart_canvas.width;
    let drawing_height: number = 
      this.chart_canvas.height-this.PADDING_BOT;

    ctx.strokeStyle = 'white';
    ctx.textBaseline = 'middle';

    ctx.fillStyle = 'black'; 
    ctx.fillRect(0,0,this.chart_canvas.width, this.chart_canvas.height);
    ctx.fillStyle = 'white';

    let candles: Chart[] = chart.chart;

    let num_displayable_candles: number = 
      (this.chart_canvas.width-this.getPriceWidth(ctx))/this.CANDLE_WIDTH;
    num_displayable_candles -= 1;

    let start_index: number;
    if (num_displayable_candles < candles.length)
      start_index = Math.round(candles.length-num_displayable_candles);
    else
      start_index = 0;


    let priceRange: ChartRange = this.getChartRange(candles, start_index);


    ctx.moveTo(drawing_width-this.getPriceWidth(ctx), 0);
    ctx.lineTo(drawing_width-this.getPriceWidth(ctx), this.chart_canvas.height);
    ctx.stroke();

    let pixel_to_price: LinearEquation = 
      new LinearEquation(0, priceRange.max, drawing_height, priceRange.min);

    let price_to_pixel: LinearEquation = 
      new LinearEquation(priceRange.min, drawing_height, priceRange.max,
                        this.PADDING_TOP);


    // DRAW THE PRICE TICKS
    let inc: number = (priceRange.max-priceRange.min)/this.NUM_TICKS;
    for (let i: number = priceRange.min; i <= priceRange.max; i += inc) {
      ctx.fillText("-" + (i/10000).toFixed(4),
                   drawing_width-this.getPriceWidth(ctx),
                   price_to_pixel.eval(i));
      //ctx.beginPath();
      //ctx.moveTo(0, price_to_pixel.eval(i));
      //ctx.lineTo(drawing_width-this.getPriceWidth(ctx), price_to_pixel.eval(i));
      //ctx.stroke();
    }

    // DRAW THE GRID LINES
    //for (let i: number = 0; i < drawing_width-this.getPriceWidth(ctx);
    //     i += this.CANDLE_WIDTH) {
    //  ctx.beginPath(); 
    //  ctx.moveTo(i,0);
    //  ctx.lineTo(i, this.chart_canvas.height);
    //  ctx.stroke();
    //}

    // DRAW THE CANDLES
    for (let i: number = start_index; i < candles.length; ++i) {
      let width_offset: number = 
        ((i-start_index)*this.CANDLE_WIDTH);
      
      ctx.fillStyle = 'blue';
      ctx.beginPath();
      ctx.moveTo(width_offset + this.CANDLE_WIDTH/2.0,
                 price_to_pixel.eval(candles[i].candle.h));
      ctx.lineTo(width_offset + this.CANDLE_WIDTH/2.0,
                 price_to_pixel.eval(candles[i].candle.l));
      ctx.stroke();

      if (candles[i].candle.o > candles[i].candle.c) {
        ctx.fillStyle = 'blue';
        ctx.fillRect(
          width_offset,
          price_to_pixel.eval(candles[i].candle.c),
          this.CANDLE_WIDTH,
          price_to_pixel.eval(candles[i].candle.o)-
            price_to_pixel.eval(candles[i].candle.c));
        
        ctx.fillStyle = 'black';
      }
      if (candles[i].candle.o < candles[i].candle.c) {
        ctx.fillStyle = 'yellow';
        ctx.fillRect(
          width_offset,
          price_to_pixel.eval(candles[i].candle.o),
          this.CANDLE_WIDTH,
          price_to_pixel.eval(candles[i].candle.c)-
            price_to_pixel.eval(candles[i].candle.o));
        ctx.fillStyle = 'black';
      }
    }

    this.conn.send('init|' + this.symbol);
  }

}
