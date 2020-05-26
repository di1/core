/**
  The view representing the candles plane
 */
class ChartCandleView { // eslint-disable-line no-unused-vars
  // The canvas dom element and renderer
  private CanvasElement: HTMLCanvasElement;
  private Renderer: CanvasRenderingContext2D;

  // The candle to start drawing from and the number of candles
  // that can be drawn on the canvas
  private StartCandle: number;
  private NumVisibleCandles: number;

  // The drawing width and height
  private Width: number;
  private Height: number;

  // The current range of the chart
  private MaxCandleValue: number;
  private MinCandleValue: number;

  private CandleWidth: number;
  private CandleSpacing: number;

  private PixelRatio: number;

  // Private most up to date chart
  private FullChartData: IChart;

  // Draws a horizontal line every 15 minutes by default
  // this is default because we can only display minute candle data
  // (the only useful data)
  private HorizontalLineInterval: number = 15;

  private CandleViewWidthOffset: number;

  private Precision: number = 100000.0;
  private FixedCount: number = 5;
  /**
    Default constructor
    @param {HTMLCanvasElement} canvaselement The <canvas> element to draw to
    @param {IChart} cht The initial full chart
   */
  constructor(canvaselement: HTMLCanvasElement, cht: IChart) {
    this.CanvasElement = canvaselement;
    this.Renderer =
      this.CanvasElement.getContext('2d') as CanvasRenderingContext2D;

    this.StartCandle = 0;
    this.NumVisibleCandles = 0;
    this.Width = 0;
    this.CandleViewWidthOffset = 68;
    this.Height = 0;
    this.MaxCandleValue = 0;
    this.MinCandleValue = 0;
    this.CandleWidth = 4;
    this.CandleSpacing = 2;

    // Need to scale for high resolution displays
    this.PixelRatio = window.devicePixelRatio;

    // The initial full chart to draw
    this.FullChartData = cht;

    // start the drawing loop
    this.draw();
  }

  /**
    Converts the fixed point number into nice looking text with
    decimal point
    @param {number} price The price to change
    @return {string} The string representation of the number
   */
  private priceToText(price: number): string {
    return ' ' + (price / this.Precision).toFixed(this.FixedCount);
  }

  /**
    Condition the frame for redraw, this also starts the drawing loop
   */
  private draw(): void {
    // clear everything
    this.Renderer.save();

    this.Width = this.CanvasElement.getBoundingClientRect().width;
    this.Height = this.CanvasElement.getBoundingClientRect().height;

    this.Renderer.clearRect(0, 0, this.Width, this.Height);

    this.CanvasElement.width = Math.floor(this.Width * this.PixelRatio);
    this.CanvasElement.height = Math.floor(this.Height * this.PixelRatio);

    this.Width -= this.CandleViewWidthOffset;

    this.Renderer.scale(this.PixelRatio, this.PixelRatio);

    // Compute the number of visible candles. We floor because
    // a floating point number will cause visual problems
    const numberVisibleCandles: number =
      Math.floor((this.Width / (this.CandleWidth + this.CandleSpacing)));

    // Determine the left most candle index. If we can't fit everything
    // we will have to change the left most candle starting index
    let lftCndIdx: number = 0;
    if (numberVisibleCandles < this.FullChartData.chart.length) {
      lftCndIdx = this.FullChartData.chart.length - numberVisibleCandles;
    }

    // Now we will draw a grid of vertical lines to represent the spaces
    // between the candles.
    this.drawHorizontalGrid();

    // Compute the transformation function to convert candle price data
    // to screen values.
    const [pmin, pmax] = this.chartMinMax(lftCndIdx);

    // Create the price transformation to convert prices into pixel
    // location
    const pt: LinearEquation = new LinearEquation(pmax, 0, pmin, this.Height);

    // Draw a vertical line at every 10 units (1 pip for most)
    this.drawVerticalGrid(pt, pmin, pmax);

    // Draw the candles
    this.drawCandles(pt, lftCndIdx);

    this.Renderer.restore();
  }

  /**
    Updates the chart based on the latest candle info.
    If a new candle is needed, will return false which will trigger
    a full chart update.
    @param {ILatestCandle} cnd The latest candle that was received
    @return {boolean} True if the update was ok, false if a full chart request
    is needed.
   */
  public chartPartialUpdate(cnd: ILatestCandle): boolean {
    const chtLgt: number = this.FullChartData.chart.length;
    const lstCnd: Candle = this.FullChartData.chart[chtLgt-1].candle;
    if (lstCnd.s == cnd.latestCandle.candle.s) {
      this.FullChartData.chart[chtLgt-1].candle = cnd.latestCandle.candle;

      if (lstCnd.o !== cnd.latestCandle.candle.o ||
          lstCnd.h !== cnd.latestCandle.candle.h ||
          lstCnd.l !== cnd.latestCandle.candle.l ||
          lstCnd.c !== cnd.latestCandle.candle.c) {
        this.draw();
      }
      return true;
    } else {
      return false;
    }
  }

  /**
    Pushes a full chart update refreshing the entire candles
    @param {IChart} cht The new full chart
   */
  public chartfullUpdate(cht: IChart): void {
    this.FullChartData = cht;
    this.draw();
  }

  /**
    Computes the minimum and maximum values for the chart
    range given.
    @param {number} lftCndIdx The left most candle index to start at.
    @return {[number, number]} The first number is the minimum and the second
    is the maximum.
   */
  private chartMinMax(lftCndIdx: number): [number, number] {
    let gmin: number = Number.MAX_VALUE;
    let gmax: number = Number.MIN_VALUE;

    const chtLength: number = this.FullChartData.chart.length;
    for (let i: number = lftCndIdx; i < chtLength; ++i) {
      if (this.FullChartData.chart[i].candle.h > gmax) {
        gmax = this.FullChartData.chart[i].candle.h;
      }
      if (this.FullChartData.chart[i].candle.l < gmin) {
        gmin = this.FullChartData.chart[i].candle.l;
      }
    }
    return [gmin, gmax];
  }

  /**
    Draws a price tick every 10 units.
    @param {LinearEquation} pt The transformation to be used
    @param {number} pmin The minimum price that will be displayed
    @param {number} pmax The maximum price that will be displayed
   */
  private drawVerticalGrid(pt: LinearEquation, pmin: number,
      pmax: number): void {
    this.Renderer.save();
    this.Renderer.strokeStyle = '#B3B1AD';
    this.Renderer.fillStyle = '#B3B1AD';
    this.Renderer.lineWidth = 0.3;

    this.Renderer.font = '16px Inconsolata';
    this.Renderer.textBaseline = 'middle';

    const numTicks: number = 20.0;
    const tickInc: number = (pmax - pmin) / numTicks;
    this.Renderer.beginPath();
    for (let i: number = pmin; i <= pmax; i += tickInc) {
      const ylvl: number = pt.eval(i);
      this.Renderer.moveTo(0.5, ylvl + 0.5);
      this.Renderer.lineTo(this.Width + 2.5, ylvl + 0.5);

      if (i == pmin) {
        this.Renderer.textBaseline = 'bottom';
      } else if ((i + tickInc) >= pmax) {
        this.Renderer.textBaseline = 'hanging';
      } else {
        this.Renderer.textBaseline = 'middle';
      }

      this.Renderer.textAlign = 'left';
      this.Renderer.fillText(this.priceToText(i),
          this.Width,
          ylvl + 0.5);
    }
    this.Renderer.stroke();
    this.Renderer.restore();
  }

  /**
    Draws horizontal lines marking the candles
   */
  private drawHorizontalGrid(): void {
    this.Renderer.save();
    this.Renderer.strokeStyle = '#B3B1AD';
    this.Renderer.lineWidth = 0.3;
    this.Renderer.beginPath();
    for (let i: number = 0; i <= this.Width;
      i += ((this.CandleWidth + this.CandleSpacing)*
               this.HorizontalLineInterval)) {
      this.Renderer.moveTo(i+0.5, 0.5);
      this.Renderer.lineTo(i+0.5, this.Height+0.5);
    }

    // draw the last line for the price bar
    this.Renderer.moveTo(this.Width + 0.5, 0.5);
    this.Renderer.lineTo(this.Width + 0.5, this.Height+0.5);
    this.Renderer.stroke();
    this.Renderer.restore();
  }

  /**
    Draws the candles to canvas
    @param {LinearEquation} pt The price transformations
    @param {number} lftCndIdx The starting index
   */
  private drawCandles(pt: LinearEquation, lftCndIdx: number): void {
    this.Renderer.save();

    const chtLength: number =
      this.FullChartData.chart.length;
    let curCnd: Candle | null = null;
    for (let i: number = lftCndIdx; i < chtLength; ++i) {
      curCnd = this.FullChartData.chart[i].candle;
      const adjidx: number = i - lftCndIdx;

      this.Renderer.beginPath();

      // compute the drawing color based on if the candle is going up or down
      const drawColor: string = (curCnd.o > curCnd.c) ? '#FF3333' : '#BAE67E';
      this.Renderer.strokeStyle = drawColor;
      this.Renderer.fillStyle = drawColor;
      // compute the height
      const hght: number = (curCnd.o > curCnd.c) ?
        pt.eval(curCnd.c) - pt.eval(curCnd.o) :
        pt.eval(curCnd.o) - pt.eval(curCnd.c);
      // compute the top left
      const tl: number = (curCnd.o > curCnd.c) ?
        pt.eval(curCnd.o) : pt.eval(curCnd.c);

      // how far we are from the left
      const xoffset: number =
        (adjidx * (this.CandleWidth + this.CandleSpacing) +
        this.CandleSpacing / 2.0);

      // draw a rectangle if the height isn't 0 otherwise draw a line
      if (curCnd.o != curCnd.c) {
        this.Renderer.fillRect(xoffset, tl, this.CandleWidth, hght);
      } else {
        this.Renderer.moveTo(xoffset, tl);
        this.Renderer.lineTo(xoffset + this.CandleWidth, tl);
      }

      this.Renderer.moveTo(((xoffset) + this.CandleWidth / 2.0) + 0.5,
          pt.eval(curCnd.h) + 0.5);
      this.Renderer.lineTo(((xoffset) + this.CandleWidth / 2.0) + 0.5,
          pt.eval(curCnd.l));

      this.Renderer.strokeStyle = drawColor;
      this.Renderer.stroke();
    }

    // draw the price bar information about the current candle
    if (curCnd) {
      const ylvl: number = pt.eval(curCnd.c);
      this.Renderer.fillRect(this.Width+2.5, ylvl-8, this.CandleViewWidthOffset,
          16);

      if (this.Renderer.fillStyle == '#FF3333') {
        this.Renderer.fillStyle = '#3c3c3c';
      } else {
        this.Renderer.fillStyle = '#481e81';
      }
      this.Renderer.textAlign = 'left';
      this.Renderer.textBaseline = 'middle';
      this.Renderer.font = '16px Inconsolata';
      this.Renderer.fillText(this.priceToText(curCnd.c),
          this.Width,
          ylvl + 0.5);
    }
    this.Renderer.restore();
  }
}
