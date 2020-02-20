/**
  Represents a CandleChart
 */
class CandleChart { // eslint-disable-line no-unused-vars
  private symbol: string;
  private conn: WebSocket;
  private chartCanvas: HTMLCanvasElement;

  private NUM_TICKS: number = 20;
  private PADDING_BOT: number = 50;
  private PADDING_TOP: number = 20;
  private CANDLE_WIDTH: number = 10;


  private ROOT_CHART: RootChartObject;
  private CANDLE_UPDATES_SENT: number = 0;
  private CANDLE_SPACING: number = 5;

  private MOUSE_X: number = 0;
  private MOUSE_Y: number = 0;

  private RESET_CHART: boolean = false;

  private ANALYSIS_RESULTS: RootAnalysisJSON;

  private CHART_STYLE_BACKGROUND_COLOR = '#131722';

  /**
    Creates a candle chart bound to the largeCandleChart.

    @param {string} symbol The security ticker; e.g AAPL
   */
  constructor(symbol: string) {
    this.chartCanvas = <HTMLCanvasElement> document.getElementById('chart');

    this.symbol = symbol;
    this.conn = new WebSocket('ws://localhost:7681', 'lws-minimal');
    this.conn.onopen = this.onOpen.bind(this);
    this.conn.onclose = this.onClose.bind(this);
    this.conn.onmessage = this.onMessage.bind(this);

    this.chartCanvas.onwheel = this.onMouseWheelEvent.bind(this);
    this.chartCanvas.onmousemove = this.onMouseMove.bind(this);

    this.ROOT_CHART = <RootChartObject> {
      chart: [],
    };

    this.ANALYSIS_RESULTS = <RootAnalysisJSON> {
      analysis: <AnalysisJSON> {singleCandle: [], trendLines: []},
    };
  }

  /**
    Sets the symbol this chart is currently displaying

    @param {string} symbol The security ticker; e.g AAPL
   */
  public setSymbol(symbol: string) {
    this.symbol = symbol;
    this.RESET_CHART = true;
  }

  /**
    Callback when a message from the websocket server is sent.

    @param {MessageEvent} evt The message event
   */
  private onMessage(evt: MessageEvent) {
    if (this.RESET_CHART) {
      this.RESET_CHART = false;
      this.conn.send('init|' + this.symbol);
      return;
    }

    const genericMsg = JSON.parse(evt.data);

    if (genericMsg['chart']) {
      const latestChart: RootChartObject | undefined = genericMsg;

      if (!latestChart) {
        console.error('onMessage latestChart == undefined');
        return;
      }

      this.ROOT_CHART = latestChart;
      this.drawFull(this.ROOT_CHART);
    } else if (genericMsg['latestCandle']) {
      const updateCandle: LatestChartCandle | undefined = genericMsg;

      if (!updateCandle) {
        console.error('onMessage updateCandle == undefined');
        return;
      }
      if (!this.ROOT_CHART) {
        console.error('onMessage updateCandle happened before init');
        return;
      }

      if (this.ROOT_CHART.chart[this.ROOT_CHART.chart.length-1].candle.s ==
          updateCandle.latestCandle.candle.s) {
        this.ROOT_CHART.chart[this.ROOT_CHART.chart.length-1].candle =
          updateCandle.latestCandle.candle;
        this.drawFull(this.ROOT_CHART);
      } else {
        this.ROOT_CHART.chart.push({candle: updateCandle.latestCandle.candle});
        this.conn.send('analysis|' + this.symbol);
        this.drawFull(this.ROOT_CHART);
      }
    } else if (genericMsg['analysis']) {
      this.ANALYSIS_RESULTS = <RootAnalysisJSON> genericMsg;
    } else {
      console.error('invalid response from server');
    }
  }

  /**
    The callback when the web socket connection has
    successfully opened

    @param {Event} evt A generic event
   */
  private onOpen(evt: Event) {
    this.conn.send('init|' + this.symbol);
  }

  /**
    The callback when the web socket connection
    closed.

    @param {CloseEvent} evt The close event
   */
  private onClose(evt: CloseEvent) {
    console.log('websocket connection closed');
  }

  /**
    Callback when the mouse wheel gets spun/moved. Will increase the candle
    width and zoom in to the right on forward spin, and decrease and zoom out
    while spun backwards.

    @param {WheelEvent} evt Thte mouse wheel event
   */
  private onMouseWheelEvent(evt: WheelEvent) {
    if (evt.deltaY > 0) {
      this.CANDLE_WIDTH += 1;
    }
    if (evt.deltaY < 0) {
      this.CANDLE_WIDTH -= 0.5;
    }
  }


  /**
    Callback when a mouse moves, updates a class specific mouse variables
    to keep track of where the mouse is.

    @param {MouseEvent} evt A mouse event
   */
  private onMouseMove(evt: MouseEvent) {
    if (!this.chartCanvas) {
      return;
    }
    const rect = this.chartCanvas.getBoundingClientRect();
    const x = evt.clientX;
    const y = evt.clientY;

    // relationship bitmap vs. element for X
    const scaleX: number = this.chartCanvas.width / rect.width;

    // relationship bitmap vs. element for Y
    const scaleY: number = this.chartCanvas.height / rect.height;

    this.MOUSE_X = (x - rect.left) * scaleX;
    this.MOUSE_Y = (y - rect.top) * scaleY;
  }

  /**
    Obtains the min, and max for the price and volume of the given data.

    @param {Chart[]} candles The full list of candles in the chart
    @param {number} startIndex A candle index to start finding the ranges
    @return {ChartRange} Returns a chartrange interface
   */
  private getChartRange(candles: Chart[], startIndex: number): ChartRange {
    let gmax: number = Number.MIN_VALUE;
    let gmin: number = Number.MAX_VALUE;
    let vgmax: number = Number.MIN_VALUE;

    for (let i: number = startIndex; i < candles.length; ++i) {
      const lmax: number = Math.max(candles[i].candle.o, candles[i].candle.h,
          candles[i].candle.l, candles[i].candle.c);
      const lmin: number = Math.min(candles[i].candle.o, candles[i].candle.h,
          candles[i].candle.l, candles[i].candle.c);

      const vlmax: number = candles[i].candle.v;

      if (lmax > gmax) {
        gmax = lmax;
      }
      if (lmin < gmin) {
        gmin = lmin;
      }
      if (vlmax > vgmax) {
        vgmax = vlmax;
      }
    }

    const r: ChartRange = {max: gmax, min: gmin, vmin: 0, vmax: vgmax};
    return r;
  }

  /**
    Calculates the width of the text ' 0000.0000' drawn on the canvas

    @param {CanvasRenderingContext2D} ctx The rendering context
    @return {number} The width of the text in pixels
   */
  private getPriceWidth(ctx: CanvasRenderingContext2D): number {
    return ctx.measureText(' 0000.0000').width;
  }

  /**
    Sets up context variables before rendering, these are the default
    values and may be changed throught the rendering.

    @param {CanvasRenderingContext2D} ctx The rendering context
   */
  private canvasRenderContex2DSetup(ctx: CanvasRenderingContext2D) {
    // Set the canvas width and height to the screens width and height
    // and translate the entire screen by a half a pixel to get cripser lines
    ctx.canvas.width = window.innerWidth;
    ctx.canvas.height = window.innerHeight;
    ctx.translate(0.5, 0.5);

    // Set font
    ctx.font = 'normal 1.4em Monospace';

    // Most drawing of text has a baseline of middle and drawing text is
    // mainly of white text
    ctx.textBaseline = 'middle';
    ctx.strokeStyle = 'white';
    ctx.fillStyle = 'white';

    // Clear the drawing area
    ctx.save();
    ctx.fillStyle = this.CHART_STYLE_BACKGROUND_COLOR;
    ctx.fillRect(0, 0, this.chartCanvas.width, this.chartCanvas.height);
    ctx.restore();
  }

  /**
    Computes the maximum number of allowed candles to be displayed

    @param {CanvasRenderingContext2D} ctx The rendering context
    @param {number} rightBarPriceWidth Number of pixels the price bar takes.
    @return {number} The maximum number of candles allowed on the screen
   */
  private computeNumDisplayableCandles(ctx: CanvasRenderingContext2D,
      rightBarPriceWidth: number): number {
    // Divide the chart width not include the leftBarPriceBarWidth
    // by the amount of space one candle occupies to get the maximum
    // number of candles displayable on the screen
    let numDisplayableCandles: number =
      (this.chartCanvas.width-rightBarPriceWidth) /
      (this.CANDLE_WIDTH + this.CANDLE_SPACING);

    // Subtract one candle for spacing given to the right most bar
    numDisplayableCandles -= 1;

    return numDisplayableCandles;
  }

  /**
    Calculates what the start index of the left most candle is. This number
    changes based on zoom, and chart position.

    @param {Chart[]} candles The array of candles that can be drawn.
    @param {number} maxDrawableCandles The maximum number of candles that can
                                        be drawn on the screen.
    @return {number} The start index of the left most candle
   */
  private computeCandleStartIndex(candles: Chart[],
      maxDrawableCandles: number): number {
    let startIndex: number;
    if (maxDrawableCandles < candles.length) {
      startIndex = Math.floor(candles.length-maxDrawableCandles);
    } else {
      startIndex = 0;
    }
    return startIndex;
  }

  /**
    Draws the right price bar.

    @param {CanvasRenderingContext2D} ctx
    @param {number} drawingWidth The width of the entire canvas.
    @param {number} rightBarPriceWidth The alloted space for the price bar.
    @param {ChartRange} priceRange The price range of the chart
    @param {LinearEquation} priceToPixel An equation that converts a price
      to a Y pixel coordiant.
   */
  private drawRightPriceBar(ctx: CanvasRenderingContext2D,
      drawingWidth: number, rightBarPriceWidth: number, priceRange: ChartRange,
      priceToPixel: LinearEquation) {
    ctx.save();
    ctx.translate(drawingWidth-rightBarPriceWidth, 0);

    ctx.beginPath();
    ctx.moveTo(0, 0);
    ctx.lineTo(0, this.chartCanvas.height);
    ctx.stroke();

    // Draw the price tick follwed by the price it coorisponds to
    const inc: number = (priceRange.max-priceRange.min)/this.NUM_TICKS;
    for (let i: number = priceRange.min; i <= priceRange.max; i += inc) {
      ctx.fillText('-' + (i/10000).toFixed(4), 0, priceToPixel.eval(i));
    }

    ctx.restore();
  }

  /**
    Draws the chart on the html canvas

    @param {RootChartObject} chart The interface representing the candles
   */
  private drawFull(chart: RootChartObject) {
    // Get the drawing context for the canvas
    const ctx: CanvasRenderingContext2D =
      <CanvasRenderingContext2D> this.chartCanvas.getContext('2d');

    // Setup the context defaults and clear the canvas
    this.canvasRenderContex2DSetup(ctx);

    // The drawing width and height of the candle chart
    // (not including the bottom volume graph)
    const drawingWidth: number = this.chartCanvas.width;
    const drawingHeight: number =
      this.chartCanvas.height-this.PADDING_BOT;

    // The amount of space needed to draw the right price bar
    const rightBarPriceWidth: number = ctx.measureText(' 0000.0000').width;

    // Hold a variable to store the candles to reduce typing of
    // chart.chart
    const candles: Chart[] = chart.chart;

    // Maximum number of candles allowed to be displayed
    const numDisplayableCandles: number =
      this.computeNumDisplayableCandles(ctx, rightBarPriceWidth);

    // The start index of the left most candle
    const startIndex: number = this.computeCandleStartIndex(candles,
        numDisplayableCandles);

    // The price range that the chart covers
    const priceRange: ChartRange = this.getChartRange(candles, startIndex);

    // A linear equation when given a pixel will give back the price on the
    // chart.
    const pixelToPrice: LinearEquation =
      new LinearEquation(this.PADDING_TOP, priceRange.max,
          drawingHeight-this.PADDING_BOT, priceRange.min);

    // A linear equation when given a price will give back the pixel location
    // on the chart.
    const priceToPixel: LinearEquation =
      new LinearEquation(priceRange.max, this.PADDING_TOP, priceRange.min,
          drawingHeight);

    // A linear equation when given a volume will give back the pixel location
    // on the chart.
    const volumeToPixel: LinearEquation =
      new LinearEquation(priceRange.vmin, this.chartCanvas.height,
          priceRange.vmax, drawingHeight);


    // Draw the right price bar
    this.drawRightPriceBar(ctx, drawingWidth, rightBarPriceWidth, priceRange,
        priceToPixel);

    // DRAW THE CANDLES
    let lastColor: string = '';
    for (let i: number = startIndex; i < candles.length; ++i) {
      const widthOffset: number =
        ((i-startIndex)*(this.CANDLE_WIDTH+this.CANDLE_SPACING));

      let hasAnalysis: boolean = false;
      if (this.ANALYSIS_RESULTS &&
          i < this.ANALYSIS_RESULTS.analysis.singleCandle.length &&
          this.ANALYSIS_RESULTS.analysis.singleCandle[i] != 0) {
        hasAnalysis = true;
      }

      if (candles[i].candle.o > candles[i].candle.c) {
        ctx.fillStyle = '#EF5350';
        ctx.strokeStyle = ctx.fillStyle;
        lastColor = ctx.fillStyle;

        if (hasAnalysis) {
          ctx.strokeRect(
              widthOffset,
              priceToPixel.eval(candles[i].candle.c),
              this.CANDLE_WIDTH,
              priceToPixel.eval(candles[i].candle.o)-
              priceToPixel.eval(candles[i].candle.c));
        } else {
          ctx.fillRect(
              widthOffset,
              priceToPixel.eval(candles[i].candle.c),
              this.CANDLE_WIDTH,
              priceToPixel.eval(candles[i].candle.o)-
              priceToPixel.eval(candles[i].candle.c));
        }

        ctx.beginPath();
        ctx.moveTo(widthOffset + this.CANDLE_WIDTH/2.0,
            priceToPixel.eval(candles[i].candle.h));
        ctx.lineTo(widthOffset + this.CANDLE_WIDTH/2.0,
            priceToPixel.eval(candles[i].candle.o));
        ctx.moveTo(widthOffset + this.CANDLE_WIDTH/2.0,
            priceToPixel.eval(candles[i].candle.l));
        ctx.lineTo(widthOffset + this.CANDLE_WIDTH/2.0,
            priceToPixel.eval(candles[i].candle.c));
        ctx.stroke();

        ctx.beginPath();
        ctx.fillRect(widthOffset,
            volumeToPixel.eval(candles[i].candle.v),
            this.CANDLE_WIDTH,
            this.chartCanvas.height-volumeToPixel.eval(candles[i].candle.v));
        ctx.stroke();


        ctx.fillStyle = 'black';
      } else if (candles[i].candle.o < candles[i].candle.c) {
        ctx.fillStyle = '#26A69A';
        ctx.strokeStyle = ctx.fillStyle;
        lastColor = ctx.fillStyle;

        if (hasAnalysis) {
          ctx.strokeRect(
              widthOffset,
              priceToPixel.eval(candles[i].candle.o),
              this.CANDLE_WIDTH,
              priceToPixel.eval(candles[i].candle.c)-
              priceToPixel.eval(candles[i].candle.o));
        } else {
          ctx.fillRect(
              widthOffset,
              priceToPixel.eval(candles[i].candle.o),
              this.CANDLE_WIDTH,
              priceToPixel.eval(candles[i].candle.c)-
              priceToPixel.eval(candles[i].candle.o));
        }

        ctx.beginPath();
        ctx.moveTo(widthOffset + this.CANDLE_WIDTH/2.0,
            priceToPixel.eval(candles[i].candle.h));
        ctx.lineTo(widthOffset + this.CANDLE_WIDTH/2.0,
            priceToPixel.eval(candles[i].candle.c));
        ctx.moveTo(widthOffset + this.CANDLE_WIDTH/2.0,
            priceToPixel.eval(candles[i].candle.l));
        ctx.lineTo(widthOffset + this.CANDLE_WIDTH/2.0,
            priceToPixel.eval(candles[i].candle.o));
        ctx.stroke();

        ctx.beginPath();
        ctx.fillRect(widthOffset,
            volumeToPixel.eval(candles[i].candle.v),
            this.CANDLE_WIDTH,
            this.chartCanvas.height-volumeToPixel.eval(candles[i].candle.v));
        ctx.stroke();

        ctx.fillStyle = 'black';
      } else {
        if (candles[i].candle.v != 0) {
          ctx.fillStyle = 'white';
          ctx.strokeStyle = 'white';
          ctx.beginPath();
          ctx.moveTo(widthOffset,
              priceToPixel.eval(candles[i].candle.o));
          ctx.lineTo(widthOffset + this.CANDLE_WIDTH,
              priceToPixel.eval(candles[i].candle.c));
          ctx.stroke();

          ctx.beginPath();
          ctx.moveTo(widthOffset + this.CANDLE_WIDTH/2.0,
              priceToPixel.eval(candles[i].candle.h));
          ctx.lineTo(widthOffset + this.CANDLE_WIDTH/2.0,
              priceToPixel.eval(candles[i].candle.c));
          ctx.moveTo(widthOffset + this.CANDLE_WIDTH/2.0,
              priceToPixel.eval(candles[i].candle.l));
          ctx.lineTo(widthOffset + this.CANDLE_WIDTH/2.0,
              priceToPixel.eval(candles[i].candle.o));
          ctx.stroke();


          ctx.beginPath();
          ctx.fillRect(widthOffset,
              volumeToPixel.eval(candles[i].candle.v),
              this.CANDLE_WIDTH,
              this.chartCanvas.height-volumeToPixel.eval(candles[i].candle.v));
          ctx.stroke();
        }
      }

      if (this.ANALYSIS_RESULTS != undefined &&
          i < this.ANALYSIS_RESULTS.analysis.singleCandle.length &&
          this.ANALYSIS_RESULTS.analysis.singleCandle[i] != 0) {
        ctx.textBaseline = 'top';
        ctx.fillStyle = 'white';

        const candleCode: number =
          this.ANALYSIS_RESULTS.analysis.singleCandle[i];
        let candleId: string;

        switch (candleCode) {
          case 0:
            candleId = '';
            break;
          case 1:
          case 2:
            candleId = 'M';
            break;
          case 3:
          case 4:
            candleId = 'S';
            break;
          case 5:
          case 6:
          case 7:
            candleId = 'D';
            break;
          default:
            console.error('i don\'t know this candle pattern');
            candleId = 'X';
            break;
        }

        ctx.font = 'normal ' +
          (this.CANDLE_WIDTH*2.0).toString() + 'px monospace';

        ctx.fillText(candleId,
            (widthOffset + (this.CANDLE_WIDTH/2.0)) -
              ctx.measureText(candleId).width/2.0,
            priceToPixel.eval(candles[i].candle.l) + 5);

        ctx.textBaseline = 'middle';
        ctx.font = 'normal 1.4em Monospace';
      }
    }

    // Draw the trend lines
    if (this.ANALYSIS_RESULTS != undefined &&
        this.ANALYSIS_RESULTS.analysis.trendLines.length != 0) {
      for (let i: number = 0;
        i < this.ANALYSIS_RESULTS.analysis.trendLines.length; ++i) {
        const trend: TrendLine = this.ANALYSIS_RESULTS.analysis.trendLines[i];
        if (trend.s >= startIndex) {
          ctx.beginPath();
          ctx.save();
          ctx.lineWidth = 0.5;
          ctx.strokeStyle = 'yellow';
          ctx.fillStyle = 'yellow';
          if (trend.d) {
            ctx.moveTo(
                (trend.s-startIndex)*(this.CANDLE_WIDTH+this.CANDLE_SPACING),
                priceToPixel.eval(candles[trend.e].candle.h));
            ctx.lineTo(
                (trend.e-startIndex+1)*(this.CANDLE_WIDTH+this.CANDLE_SPACING),
                priceToPixel.eval(candles[trend.e].candle.h));
          }
          ctx.stroke();
          ctx.restore();
          ctx.strokeStyle = 'black';
        }
      }
    }

    ctx.fillStyle = lastColor;
    ctx.fillRect(drawingWidth-this.getPriceWidth(ctx),
        priceToPixel.eval(candles[candles.length-1].candle.c) - (20.0/2.0),
        this.getPriceWidth(ctx), 20);
    ctx.fillStyle = 'white';
    ctx.fillText(
        '-' + ((candles[candles.length-1].candle.c)/10000.0).toFixed(4),
        drawingWidth-this.getPriceWidth(ctx),
        priceToPixel.eval(candles[candles.length-1].candle.c));

    ctx.fillStyle = 'white';
    ctx.fillRect(drawingWidth-this.getPriceWidth(ctx),
        this.MOUSE_Y - (20.0/2.0),
        this.getPriceWidth(ctx), 20);
    ctx.fillStyle = 'black';
    ctx.fillText(
        '-' + ((pixelToPrice.eval(this.MOUSE_Y))/10000).toFixed(4),
        drawingWidth-this.getPriceWidth(ctx),
        this.MOUSE_Y);

    ctx.font = (this.PADDING_TOP+1).toString() + 'px Monospace';
    ctx.fillStyle = 'white';
    ctx.textBaseline = 'top';
    ctx.fillText(this.symbol.toUpperCase(), 0, 0);

    // CONERT MOUSE COORDIANTES TO CANDLE INDEX
    let mouseCandleIndex =
      Math.floor(this.MOUSE_X / (this.CANDLE_WIDTH+this.CANDLE_SPACING));

    mouseCandleIndex += startIndex;
    if (!this.ROOT_CHART) {
      console.error('this.ROOT_CHART undefined');
      return;
    }

    if (mouseCandleIndex >= this.ROOT_CHART.chart.length) {
      mouseCandleIndex = this.ROOT_CHART.chart.length-1;
    }

    const hoveredCandle: Candle =
      this.ROOT_CHART.chart[mouseCandleIndex].candle;
    ctx.fillText(' O:' + (hoveredCandle.o/10000).toFixed(4) +
                 ' H:' + (hoveredCandle.h/10000).toFixed(4) +
                 ' L:' + (hoveredCandle.l/10000).toFixed(4) +
                 ' C:' + (hoveredCandle.c/10000).toFixed(4),
    ctx.measureText(this.symbol).width + this.PADDING_TOP,
    0);

    ctx.fillText(
        new Date(hoveredCandle.e/1000000).toDateString(),
        drawingWidth -
        ctx.measureText(new Date(hoveredCandle.e/1000000).toDateString())
            .width - this.getPriceWidth(ctx),
        0);

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
