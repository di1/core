type RectDrawFunc = (x: number, y: number, w: number, h: number) => void;

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
  private CHART_STYLE_CANDLE_FALLING = '#EF5350';
  private CHART_STYLE_CANDLE_RISING = '#26A69A';

  /**
    Resets all the top level data that determines the chart
   */
  private resetChart() {
    this.ROOT_CHART = <RootChartObject> {
      chart: [],
    };

    this.ANALYSIS_RESULTS = <RootAnalysisJSON> {
      analysis: <AnalysisJSON> {singleCandle: [], trendLines: []},
    };
  }
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
      this.resetChart();
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
    // Save the context state so we don't mess anything else up
    ctx.save();

    // Translate to the  price bar so our drawing is 0 based
    // set line width to 0.5
    ctx.translate(drawingWidth-rightBarPriceWidth, 0);
    ctx.lineWidth = 0.5;
    ctx.beginPath();
    ctx.moveTo(0, 0);
    ctx.lineTo(0, this.chartCanvas.height);
    ctx.stroke();

    // Set the line dash now for when we draw the vertical bars
    ctx.setLineDash([2, 10]);

    // Draw the price tick follwed by the price it coorisponds to
    const inc: number = (priceRange.max-priceRange.min)/this.NUM_TICKS;
    for (let i: number = priceRange.min; i <= priceRange.max; i += inc) {
      ctx.fillText('-' + (i/10000).toFixed(4), 0, priceToPixel.eval(i));

      ctx.beginPath();
      ctx.setLineDash([2, 10]);
      ctx.moveTo(-drawingWidth, priceToPixel.eval(i));
      ctx.lineTo(0, priceToPixel.eval(i));
      ctx.stroke();
    }

    // Restore back to whatever we were before
    ctx.restore();
  }

  /**
    Draws a single candle.

    @param {CanvasRenderingContext2D} ctx The canvas rendering context
    @param {number} offset The left most x coordinant of the candle
    @param {RectDrawFunc} draw A function pointer to either ctx.fillRect or
      ctx.strokeRect
    @param {Candle} candle The candle to draw
    @param {LinearEquation} priceToPixel An equation that converts a price
      to a Y pixel coordiant.
   */
  private drawCandle(ctx: CanvasRenderingContext2D,
      offset: number, draw: RectDrawFunc, candle: Candle,
      priceToPixel: LinearEquation) {
    ctx.save();
    // The candle's top left y coordiant
    const y: number = (candle.o >= candle.c) ?
      priceToPixel.eval(candle.o) : priceToPixel.eval(candle.c);

    // height from the high to the top of the candle body
    const highToY: number = priceToPixel.eval(candle.h) - y;

    // The height of the candle
    const h: number =
      Math.abs(priceToPixel.eval(candle.o)-priceToPixel.eval(candle.c));

    // height from the low to the bottom of the candle body
    const lowToBottom : number = priceToPixel.eval(candle.l) - y;

    // Set the color based on weather or not the candle is falling or rising
    ctx.strokeStyle = (candle.o > candle.c) ?
      this.CHART_STYLE_CANDLE_FALLING : this.CHART_STYLE_CANDLE_RISING;
    ctx.fillStyle = ctx.strokeStyle;

    // Translate to the top left corner of the candle
    ctx.translate(offset, y);
    ctx.beginPath();

    // call the given rect function
    draw.apply(ctx, [0, 0, this.CANDLE_WIDTH, h]);

    ctx.stroke();

    // Draw the wicks of the candle relative to the top left corner of the
    // candle.
    ctx.beginPath();
    ctx.moveTo(this.CANDLE_WIDTH / 2.0, highToY);
    ctx.lineTo(this.CANDLE_WIDTH / 2.0, 0);

    ctx.moveTo(this.CANDLE_WIDTH / 2.0, h);
    ctx.lineTo(this.CANDLE_WIDTH / 2.0, lowToBottom);
    ctx.stroke();

    ctx.restore();
  }

  /**
    Draws the volume box of a candle below candle chart

    @param {CanvasRenderingContext2D} ctx The canvas rendering context
    @param {number} offset The X offset of the candle
    @param {Candle} candle The candle object containing the volume data
    @param {LinearEquation} volumeToPixel An equation that converts volume
            to an Y coordinant.
           */
  private drawVolume(ctx: CanvasRenderingContext2D,
      offset: number, candle: Candle,
      volumeToPixel: LinearEquation) {
    ctx.save();

    // Make the stroke style the same as the candle this volume represents.
    ctx.strokeStyle = (candle.o > candle.c) ?
      this.CHART_STYLE_CANDLE_FALLING : this.CHART_STYLE_CANDLE_RISING;
    ctx.fillStyle = ctx.strokeStyle;

    ctx.translate(offset, volumeToPixel.eval(candle.v));
    ctx.beginPath();
    ctx.fillRect(0, 0, this.CANDLE_WIDTH, volumeToPixel.eval(candle.v));
    ctx.stroke();

    ctx.stroke();
    ctx.restore();
  }

  /**
    Draws an analysis that applies to a single candle

    @param {CanvasRenderingContext2D} ctx The rendering context
    @param {number} widthOffset The X offset of the candle
    @param {LinearEquation} priceToPixel An equation that converts a price
      to a Y pixel coordiant.
    @param {Candle} candle The candle to draw below
    @param {number} analysisCode The single candle analysis code number
   */
  private drawSingleCandleAnalysis(ctx: CanvasRenderingContext2D,
      widthOffset: number, priceToPixel: LinearEquation, candle: Candle,
      analysisCode: number) {
    // Before doing analysis, check to make sure the code != 0
    if (analysisCode == 0) {
      return;
    }

    ctx.save();

    // Set the baselines to draw from the top
    // and set the fill style
    ctx.textBaseline = 'top';
    ctx.fillStyle = 'white';

    // Set the font to draw the width of the candle
    ctx.font = 'normal ' +
      (this.CANDLE_WIDTH*2.0).toString() + 'px monospace';


    // Convert the analysisCode into a character for better visualization
    let candleId: string;
    switch (analysisCode) {
      case 0:
        // Nothing special
        candleId = '';
        break;
      case 1:
      case 2:
        // Marubuzu
        candleId = 'M';
        break;
      case 3:
      case 4:
        // Spinning Top
        candleId = 'S';
        break;
      case 5:
      case 6:
      case 7:
        // Doji
        candleId = 'D';
        break;
      default:
        console.error('i don\'t know this candle pattern');
        candleId = '?';
        break;
    }

    // Translate to the middle of the low of the candle
    ctx.translate(
        widthOffset + this.CANDLE_WIDTH / 2.0, priceToPixel.eval(candle.l));

    ctx.fillText(candleId, - (ctx.measureText(candleId).width/2.0), 5);

    ctx.restore();
  }

  /**
    Draws a trend line

    @param {CanvasRenderingContext2D} ctx The rendering context
    @param {Chart[]} candles The full list of candles
    @param {LinearEquation} priceToPixel An equation that converts a price
          to a Y pixel coordiant
    @param {TrendLine} trendLine The trend line object containing information
          about the trend line to draw
    @param {number} startIndex The left most draw candle index
   */
  private drawTrendLine(ctx: CanvasRenderingContext2D, candles: Chart[],
      priceToPixel: LinearEquation, trendLine: TrendLine, startIndex: number) {
    // Make sure the trend line is in a visible range
    if (trendLine.s < startIndex) {
      return;
    }

    let height: number = 0;
    if (trendLine.d == 0) { // support
      height = priceToPixel.eval(candles[trendLine.e].candle.l);
      ctx.strokeStyle = 'yellow';
    } else if (trendLine.d == 1) { // resistance
      height = priceToPixel.eval(candles[trendLine.e].candle.h);
      ctx.strokeStyle = 'lightblue';
    }

    ctx.save();
    // ctx.strokeStyle = (trendLine.d) ? 'yellow' : '#2c84e6';
    ctx.fillStyle = ctx.strokeStyle;

    const startingOffsetX: number =
      (trendLine.s-startIndex)*(this.CANDLE_WIDTH+this.CANDLE_SPACING);

    const endingOffset =
      (trendLine.e-trendLine.s)*(this.CANDLE_WIDTH+this.CANDLE_SPACING) +
      this.CANDLE_WIDTH;

    ctx.translate(startingOffsetX, height);
    ctx.beginPath();
    ctx.moveTo(0, 0);
    ctx.lineTo(endingOffset, 0);

    // Draw the arrow head

    ctx.stroke();
    ctx.restore();
  }

  /**
    Draws the scene represented by the candle chart

    @param {CanvasRenderingContext2D} ctx The rendering context
    @param {number} startIndex The start index of the candle to draw
    @param {Chart[]} candles The entire list of candles avaliable
    @param {LinearEquation} priceToPixel An equation that converts a price
      to a Y pixel coordiant.
    @param {LinearEquation} volumeToPixel An equation that converts a volume
      to a Y pixel coordiant.
   */
  private drawChart(ctx: CanvasRenderingContext2D, startIndex: number,
      candles: Chart[], priceToPixel: LinearEquation,
      volumeToPixel: LinearEquation) {
    ctx.save();

    const analysisData : AnalysisJSON = this.ANALYSIS_RESULTS.analysis;

    for (let i: number = startIndex; i < candles.length; ++i) {
      // Compute the X offset of the candle
      const widthOffset: number =
        ((i-startIndex)*(this.CANDLE_WIDTH+this.CANDLE_SPACING));

      // Extract the single candle analysis code
      const analysisCode: number = (i < analysisData.singleCandle.length) ?
        analysisData.singleCandle[i] : 0;

      // Draw the candle body and wick
      this.drawCandle(ctx, widthOffset,
        (analysisCode > 0) ? ctx.strokeRect : ctx.fillRect, candles[i].candle,
        priceToPixel);

      // Draw the single candle analysis
      this.drawSingleCandleAnalysis(
          ctx, widthOffset, priceToPixel, candles[i].candle, analysisCode);

      // Draw the volume box
      this.drawVolume(ctx, widthOffset, candles[i].candle, volumeToPixel);
    }

    // Draw the trend lines of the analysis
    for (let i: number = 0; i < analysisData.trendLines.length; ++i) {
      const trendLine: TrendLine = analysisData.trendLines[i];
      this.drawTrendLine(ctx, candles, priceToPixel, trendLine, startIndex);
    }

    ctx.restore();
  }

  /**
    Converts the mouse X coordiant into a candle index

    @param {number} startIndex The start index of the left most displayed
    candle.
    @param {Chart[]} candles The full list of candles
    @return {number} Returns the candle index of the candle or the last candle
      if the mouse is to far to the right.
     */
  private mouseToCandleIndex(startIndex: number, candles: Chart[]): number {
    let mouseCandleIndex =
      Math.floor(this.MOUSE_X / (this.CANDLE_WIDTH+this.CANDLE_SPACING));
    mouseCandleIndex += startIndex;

    if (mouseCandleIndex >= candles.length) {
      mouseCandleIndex = candles.length-1;
    }

    return mouseCandleIndex;
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

    // May be usefull in the future when mouse tracking becomes more
    // advanced.

    // const pixelToPrice: LinearEquation =
    //  new LinearEquation(this.PADDING_TOP, priceRange.max,
    //                     drawingHeight-this.PADDING_BOT, priceRange.min);

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

    const mouseCandleIndex: number =
      this.mouseToCandleIndex(startIndex, candles);

    ctx.save();
    // Draw the hovered candle data
    ctx.font = (this.PADDING_TOP+1).toString() + 'px Monospace';
    ctx.textBaseline = 'top';
    ctx.fillStyle = 'white';
    ctx.fillText(' O:' + (candles[mouseCandleIndex].candle.o/10000).toFixed(4) +
                 ' H:' + (candles[mouseCandleIndex].candle.h/10000).toFixed(4) +
                 ' L:' + (candles[mouseCandleIndex].candle.l/10000).toFixed(4) +
                 ' C:' + (candles[mouseCandleIndex].candle.c/10000).toFixed(4),
    ctx.measureText(this.symbol).width + this.PADDING_TOP+1, 0);


    // Draw the symbol ticker
    ctx.fillText(this.symbol.toUpperCase(), 0, 0);
    ctx.restore();


    // Draw the right price bar
    this.drawRightPriceBar(ctx, drawingWidth, rightBarPriceWidth, priceRange,
        priceToPixel);

    // Draws the candle chart
    this.drawChart(ctx, startIndex, candles, priceToPixel, volumeToPixel);

    // Send a new update
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
