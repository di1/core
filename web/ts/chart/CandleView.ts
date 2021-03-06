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

  // Private most up to date analysis
  // we initialize this this to its primitive JSON representation
  private FullAnalaysisData: IAnalysis = {'analysisFull': []};

  // Draws a horizontal line every 15 minutes by default
  // this is default because we can only display minute candle data
  // (the only useful data)
  private HorizontalLineInterval: number = 20;

  // How must space to leave to the right of the candle
  private CandleViewWidthOffset: number;

  // This gets set to true if a resize or major change of candles happens
  private ForceRefresh: boolean = false;

  // This gets used to calculate server ping
  private DisplayLatency: number = 0;

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

    // capture the mousewheel callback for scrolling
    this.CanvasElement.onwheel = ((evt: WheelEvent) => {
      if (evt.deltaY < 0) {
        this.CandleWidth += 2;
        this.CandleSpacing = this.CandleWidth / 2;
      } else if (evt.deltaY > 0 && this.CandleWidth > 4) {
        this.CandleWidth -= 2;
        this.CandleSpacing = this.CandleWidth / 2;
      }
      this.ForceRefresh = true;
    });

    window.onresize = ((evt: UIEvent) => {
      this.ForceRefresh = true;
    });

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
    return (price / Math.pow(10, this.FullChartData.chart.precision))
        .toFixed(this.FullChartData.chart.precision);
  }

  /**
    Condition the frame for redraw, this also starts the drawing loop
   */
  private draw(): void {
    // keep track of how long it takes to draw a frame
    const frameBeginTime: number = Date.now();

    const newWidth: number =
      this.CanvasElement.getBoundingClientRect().width;
    const newHeight: number =
      this.CanvasElement.getBoundingClientRect().height;

    this.Width = newWidth;
    this.Height = newHeight;

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
    if (numberVisibleCandles < this.FullChartData.chart.candles.length) {
      lftCndIdx =
        this.FullChartData.chart.candles.length - numberVisibleCandles;
    }

    // Now we will draw a grid of vertical lines to represent the spaces
    // between the candles.
    this.drawHorizontalGrid(lftCndIdx);

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

    // get the time it took to draw this frame
    const frameEndTime: number = Date.now();

    // compute the display latency
    this.DisplayLatency = frameEndTime - frameBeginTime;
    // console.log('Display Latency: ' + this.DisplayLatency + 'ms');
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
    const chtLgt: number = this.FullChartData.chart.candles.length;
    const lstCnd: Candle = this.FullChartData.chart.candles[chtLgt-1].candle;
    if (lstCnd.s == cnd.latestCandle.candle.s) {
      if ((this.ForceRefresh) || (lstCnd.o != cnd.latestCandle.candle.o ||
          lstCnd.h != cnd.latestCandle.candle.h ||
          lstCnd.l != cnd.latestCandle.candle.l ||
          lstCnd.c != cnd.latestCandle.candle.c)) {
        this.FullChartData.chart.candles[chtLgt-1].candle =
        cnd.latestCandle.candle;
        this.draw();
        this.ForceRefresh = false;
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

    const chtLength: number = this.FullChartData.chart.candles.length;
    for (let i: number = lftCndIdx; i < chtLength; ++i) {
      if (this.FullChartData.chart.candles[i].candle.h > gmax) {
        gmax = this.FullChartData.chart.candles[i].candle.h;
      }
      if (this.FullChartData.chart.candles[i].candle.l < gmin) {
        gmin = this.FullChartData.chart.candles[i].candle.l;
      }
    }
    if (this.FullChartData.chart.candles[chtLength - 1].candle.b < gmin) {
      gmin = this.FullChartData.chart.candles[chtLength - 1].candle.b;
    }
    if (this.FullChartData.chart.candles[chtLength - 1].candle.a > gmax) {
      gmax = this.FullChartData.chart.candles[chtLength - 1].candle.a;
    }

    const precision: number = this.FullChartData.chart.precision * 10;
    gmin -= (gmin % precision);
    gmax += (precision - (gmax % precision));
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
    this.Renderer.beginPath();
    this.Renderer.strokeStyle = '#B3B1AD';
    this.Renderer.fillStyle = '#B3B1AD';
    this.Renderer.lineWidth = 0.3;

    this.Renderer.font = '12px Inconsolata';
    this.Renderer.textBaseline = 'middle';

    // find the smallest divisor larger than precision but a factor of precision

    const precision: number = this.FullChartData.chart.precision;

    let tickInc: number = Math.floor((pmax - pmin) / 20);
    tickInc = tickInc - (tickInc % precision);

    if (tickInc == 0) {
      tickInc += precision*2;
    }

    for (let i: number = pmin;
      i <= pmax; i += tickInc) {
      if (i == pmin) {
        this.Renderer.textBaseline = 'bottom';
      } else if (i + tickInc > pmax) {
        this.Renderer.textBaseline = 'hanging';
        i = pmax;
      } else {
        this.Renderer.textBaseline = 'middle';
      }

      const ylvl: number = pt.eval(i);
      this.Renderer.moveTo(0.5, ylvl + 0.5);
      this.Renderer.lineTo(this.Width + 2.5, ylvl + 0.5);
      this.Renderer.textAlign = 'left';
      this.Renderer.fillText(' ' + this.priceToText(i),
          this.Width,
          ylvl + 0.5);
    }
    this.Renderer.closePath();
    this.Renderer.stroke();

    for (let i: number = pmin;
      i <= pmax; i += 1) {
      // draw a major axis every 5 pips to show order book levels
      if (i % 50 == 0) {
        this.drawPriceBarBox(pt, i, DarkTheme.special, DarkTheme.bg, 0.4,
            false);
      }
    }
  }

  /**
    Draws horizontal lines marking the candles
    @param {number} lftCndIdx The left most candle
   */
  private drawHorizontalGrid(lftCndIdx: number): void {
    this.Renderer.beginPath();
    this.Renderer.strokeStyle = '#B3B1AD';
    this.Renderer.lineWidth = 0.3;
    this.Renderer.beginPath();

    let i: number = lftCndIdx;
    let xoffset: number = 0;
    for (; i < this.FullChartData.chart.candles.length; ++i) {
      const cndDate: Date =
          new Date(this.FullChartData.chart.candles[i].candle.s / 1000000);
      if (cndDate.getMinutes() % 20 == 0) {
        xoffset = ((i-lftCndIdx) * (this.CandleWidth + this.CandleSpacing));
        break;
      }
    }

    while (xoffset < this.Width) {
      this.Renderer.moveTo(xoffset + 0.5, 0.5);
      this.Renderer.lineTo(xoffset + 0.5, this.Height + 0.5);
      i += 20;
      xoffset = ((i-lftCndIdx) * (this.CandleWidth + this.CandleSpacing));
    }

    this.Renderer.strokeStyle = DarkTheme.fg;
    this.Renderer.moveTo(this.Width + 0.5, 0.5);
    this.Renderer.lineTo(this.Width + 0.5, this.Height + 0.5);
    // draw the last line for the price bar
    this.Renderer.closePath();
    this.Renderer.stroke();
  }

  /**
    Draws a price box with a line spanning the chart
    @param {LinearEquation} pt The linear equation used to draw the chart
    @param {number} price The price to draw this at
    @param {string} boxFillColor The box color and line color
    @param {string} textStrokeColor The color of the text
    @param {string} lineWidth The line width to draw the line at
    @param {boolean} line Default true, if set to false will not draw line
   */
  private drawPriceBarBox(pt: LinearEquation, price: number,
      boxFillColor: string, textStrokeColor: string, lineWidth: number,
      line: boolean = true): void {
    this.Renderer.beginPath();
    let ylvl: number = pt.eval(price);
    this.Renderer.fillStyle = boxFillColor;

    if (ylvl <= 6) {
      this.Renderer.fillRect(this.Width+2.5, ylvl,
          this.CandleViewWidthOffset, 12);
    } else if (ylvl >= (this.Height - 6)) {
      ylvl = this.Height;
      this.Renderer.fillRect(this.Width+2.5, ylvl - 12,
          this.CandleViewWidthOffset, 12);
    } else {
      this.Renderer.fillRect(this.Width+2.5, ylvl-6,
          this.CandleViewWidthOffset, 12);
    }
    this.Renderer.fillStyle = textStrokeColor;
    this.Renderer.textAlign = 'left';
    this.Renderer.font = '12px Inconsolata';
    if (ylvl <= 6) {
      this.Renderer.textBaseline = 'hanging';
      this.Renderer.fillText(' ' + this.priceToText(price),
          this.Width, ylvl + 2.5);
      this.Renderer.strokeStyle = boxFillColor;
      this.Renderer.lineWidth = lineWidth;
      if (line) {
        this.Renderer.moveTo(this.Width+2.5, ylvl + 0.5);
        this.Renderer.lineTo(0, ylvl + 0.5);
      }
    } else if (ylvl >= this.Height) {
      this.Renderer.textBaseline = 'bottom';
      this.Renderer.fillText(' ' + this.priceToText(price),
          this.Width, ylvl + 0.5);
      this.Renderer.strokeStyle = boxFillColor;
      this.Renderer.lineWidth = lineWidth;
      if (line) {
        this.Renderer.moveTo(this.Width+2.5, ylvl - 0.5);
        this.Renderer.lineTo(0, ylvl - 0.5);
      }
    } else {
      this.Renderer.textBaseline = 'middle';
      this.Renderer.strokeStyle = boxFillColor;
      this.Renderer.lineWidth = lineWidth;
      if (line) {
        this.Renderer.moveTo(this.Width+2.5, ylvl + 0.5);
        this.Renderer.lineTo(0, ylvl + 0.5);
      }
      this.Renderer.fillText(' ' + this.priceToText(price),
          this.Width, ylvl + 0.5);
    }
    this.Renderer.closePath();
    this.Renderer.stroke();
  }

  /**
    Draws the candles to canvas
    @param {LinearEquation} pt The price transformations
    @param {number} lftCndIdx The starting index
   */
  private drawCandles(pt: LinearEquation, lftCndIdx: number): void {
    const chtLength: number =
      this.FullChartData.chart.candles.length;
    let curCnd: Candle | null = null;
    for (let i: number = lftCndIdx; i < chtLength; ++i) {
      curCnd = this.FullChartData.chart.candles[i].candle;
      const adjidx: number = i - lftCndIdx;
      this.drawAnalysis(pt, adjidx, lftCndIdx);

      const drawColor: string = (curCnd.o > curCnd.c) ? DarkTheme.error :
        DarkTheme.string;
      this.Renderer.beginPath();
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
        this.Renderer.stroke();
      }

      this.Renderer.moveTo(((xoffset) + this.CandleWidth / 2.0) + 0.5,
          pt.eval(curCnd.h) + 0.5);
      this.Renderer.lineTo(((xoffset) + this.CandleWidth / 2.0) + 0.5,
          pt.eval(curCnd.l));
      this.Renderer.closePath();
      this.Renderer.stroke();
    }

    // draw the price bar information about the current candle
    if (curCnd) {
      const pbars: number[] = [curCnd.a, curCnd.b];
      for (let i: number = 0; i < pbars.length; ++i) {
        this.drawPriceBarBox(pt, pbars[i], 'purple', 'white', 0.5);
      }
    }
  }

  /**
    Draws all the analysis in the analysis bin associated with this candle.
    @param {LinearEquation} pt The linear equation that is being used to
    draw the chart.
    @param {number} cndIdx The candle index adjusted for the left most candle.
    @param {number} lftCndIdx The left candle index
   */
  private drawAnalysis(pt: LinearEquation, cndIdx: number,
      lftCndIdx: number): void {
    const trueCndIdx: number = cndIdx + lftCndIdx;
    if (this.FullAnalaysisData.analysisFull[trueCndIdx] == null) {
      // nothing to draw from this candle
      return;
    }
    // not null loop through each draw function in the bin
    const analysisBin: Analysis[] =
      this.FullAnalaysisData.analysisFull[trueCndIdx] as Analysis[];
    for (let i: number = 0; i < analysisBin.length; ++i) {
      const analysis: Analysis = analysisBin[i];
      switch (analysis.type) {
        case ANALYSIS_DATA_TYPE.CANDLE_PATTERN:
          this.drawCandlePattern(analysis.data as CandlePattern,
              pt, cndIdx, lftCndIdx, i);
          break;
        case ANALYSIS_DATA_TYPE.TREND_LINE:
          this.drawTrendLine(analysis.data as TrendLine, pt, cndIdx, lftCndIdx);
          break;
      }
    }
  }

  /**
    Draws a trend line
    @param {CandlePattern} pat The candle pattern
    @param {LinearEquation} pt The linear equation used in the chart drawing.
    @param {number} cndIdx The left candle adjusted index
    @param {number} lftCndIdx The left most candle index
    @param {number} lvl The level to draw the text at
   */
  private drawTrendLine(pat: TrendLine, pt: LinearEquation, cndIdx: number,
      lftCndIdx: number): void {
    // The start and end height numbers
    let st: number = 0;
    let eh: number = 0;

    // Choose different start and end height based on numbers
    switch (pat.direction) {
      case TREND_LINE_DIRECTION.SUPPORT:
        st = pt.eval(this.FullChartData.chart.candles[pat.startIndex].candle.l);
        eh = pt.eval(this.FullChartData.chart.candles[pat.endIndex].candle.l);
        break;
      case TREND_LINE_DIRECTION.RESISTANCE:
        st = pt.eval(this.FullChartData.chart.candles[pat.startIndex].candle.h);
        eh = pt.eval(this.FullChartData.chart.candles[pat.endIndex].candle.h);
        break;
    }

    const xoffsetStart: number =
        (((pat.startIndex - lftCndIdx) *
         (this.CandleWidth + this.CandleSpacing)) + this.CandleSpacing / 2.0) +
         (this.CandleWidth / 2.0);
    const xoffsetEnd: number =
        (((pat.endIndex - lftCndIdx) *
         (this.CandleWidth + this.CandleSpacing)) + this.CandleSpacing / 2.0) +
         (this.CandleWidth / 2.0);

    this.Renderer.beginPath();
    this.Renderer.strokeStyle = '#9370DB80';
    this.Renderer.lineWidth = 2;
    this.Renderer.moveTo(xoffsetStart, st);
    this.Renderer.lineTo(xoffsetEnd, eh);
    this.Renderer.closePath();
    this.Renderer.stroke();
  }

  /**
    Draws a candle pattern
    @param {CandlePattern} pat The candle pattern
    @param {LinearEquation} pt The linear equation used in the chart drawing.
    @param {number} cndIdx The left candle adjusted index
    @param {number} lftCndIdx The left most candle index
    @param {number} lvl The level to draw the text at
   */
  private drawCandlePattern(pat: CandlePattern, pt: LinearEquation,
      cndIdx: number, lftCndIdx: number, lvl: number): void {
    // find maximum candle price
    let cndRngMax: number = Number.MIN_VALUE;
    let cndRngMin: number = Number.MAX_VALUE;

    const trueCndIdx: number = cndIdx + lftCndIdx;
    for (let i: number = trueCndIdx; i > trueCndIdx - pat.candlesSpanning;
      --i) {
      if (this.FullChartData.chart.candles[i].candle.h > cndRngMax) {
        cndRngMax = this.FullChartData.chart.candles[i].candle.h;
      }
      if (this.FullChartData.chart.candles[i].candle.l < cndRngMin) {
        cndRngMin = this.FullChartData.chart.candles[i].candle.l;
      }
    }
    // draw a shaded region covering the number of candles
    const xoffset: number =
        ((cndIdx-pat.candlesSpanning+1) *
         (this.CandleWidth + this.CandleSpacing)) + this.CandleSpacing / 2.0;

    const height: number =
      pt.eval(cndRngMax) - 0.5;

    this.Renderer.beginPath();
    this.Renderer.fillStyle = '#9370DB80';

    const drawWidth: number = (pat.candlesSpanning) *
      (this.CandleWidth + this.CandleSpacing);
    const drawHeight: number =
      pt.eval(cndRngMin) - pt.eval(cndRngMax);

    if (pat.candlesSpanning != 1) {
      this.Renderer.fillRect(xoffset - (this.CandleSpacing / 2.0), height,
          drawWidth, drawHeight+0.5);
    }

    this.Renderer.fillStyle = '#FFFFFF';
    this.Renderer.textBaseline = 'hanging';
    this.Renderer.font = (this.CandleWidth*2.0) + 'px Inconsolata';

    for (let i: number = 0; i < pat.shortCode.length; ++i) {
      this.Renderer.fillText(pat.shortCode[i],
          xoffset + ((this.CandleWidth + this.CandleSpacing)*i) + 0.5,
          pt.eval(cndRngMin) + (this.CandleWidth*2.0*lvl));
    }
    this.Renderer.closePath();
    this.Renderer.stroke();
  }

  /**
    Updates the entire analysis database and forces redraw of entire chart
    @param {IAnalysis} anl The analysis interface
   */
  public fullAnalysisUpdate(anl: IAnalysis): void {
    this.FullAnalaysisData = anl;
  }
}
