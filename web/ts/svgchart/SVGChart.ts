/**
  A class that will present an svg chart representing candle data and analysis
  from data received from a server running riski
 */
class SVGCandleChart { // eslint-disable-line no-unused-vars
  /**
    A holder for the SVG namespace url
   */
  public static SVG_NS: string = 'http://www.w3.org/2000/svg';

  /**
    The server ip to be used by the websocket
   */
  private SERVER_IP: string = 'ws://161.35.136.101:7681';

  /**
    The root svg element that holds the active chart
   */
  private SvgCandleRoot: SVGElement;

  /**
    The group containing the y axis that displays the prices on the left
   */
  private SvgYAxisRoot: SVGElement;

  /**
    The price transformation function form mapping of prices to
    svg pixel space.
   */
  private PriceTransformation: LinearEquation = new LinearEquation(0, 0, 0, 0);

  /**
    The pixel transformation function from mapping of pixels to price
   */
  private PixelTransformation: LinearEquation = new LinearEquation(0, 0, 0, 0);

  /**
    An abstraction upon the websocket that will allows for fluent
    callback communication with the server.
   */
  private Socket: SVGServerComs;

  /**
    The number of price tick markets of the right hand side
   */
  private ChartStyleNumTicks: number = 20;

  /**
    Represents the total number of candles the server has
    received
   */
  private TotalCandlesServerReceived: number = 0;

  /**
    The number of total candles the user wants the chart to display
   */
  private WantedNumberOfCandles: number = 200;

  /**
    The width of the candles
   */
  private CandleWidth: number = 10;

  /**
    Stores all the candle dom references in memory to avoid
    excessive dom quriese with document.get or document.select
    This needs to be public for the candles to access other candles
   */
  public CandleDomReferences: Array<SVGCandleStick> = [];

  /**
    Stores all the tick prices dom references
   */
  private TickPriceDomReferences: Array<SVGTextElement> = [];

  /**
    The exchange this chart will be grabbing its security from
   */
  private InputExchange: string = '';

  /**
    The security to display
   */
  private InputSecurity: string = '';

  /**
    Displays the latest candle close
   */
  private SVGLastPriceBox: SVGGElement | null = null;

  /**
    Keeps track of the number of trends draw that are sloped
   */
  private NumberOfSlopedTrendsDrawn: number = 0;

  /**
    Keeps track of the number of trends drawn that are flat
   */
  private NumberOfHorizontalTrendsDrawn: number = 0;

  /**
    Keeps track of the number of candle patterns drawn
   */
  private NumberOfCandlePatternsDrawn: number = 0;

  /**
    Creates a new live svg chart
    @param {number} svgIndex The svg index that this chart is pointing to
    @param {number} exchange The initial exchange
    @param {number} security The initial security
   */
  constructor(svgIndex: number, exchange: string, security: string) {
    this.InputExchange = exchange;
    this.InputSecurity = security;

    // Gets the root chart from the body
    const SvgCandleRootTop: SVGElement =
      <SVGElement> document.getElementsByTagNameNS(SVGCandleChart.SVG_NS,
          'svg')[svgIndex];

    this.SvgCandleRoot = <SVGElement> SvgCandleRootTop.children[0];
    this.SvgYAxisRoot = <SVGElement> SvgCandleRootTop.children[1];

    // draw the text symbols identifing what this chart is displaying
    const SvgChartTitle: SVGTextElement =
      <SVGTextElement> document.createElementNS(SVGCandleChart.SVG_NS, 'text');

    SvgChartTitle.setAttributeNS(null, 'x', '0');
    SvgChartTitle.setAttributeNS(null, 'y', '0');
    SvgChartTitle.setAttributeNS(null, 'dominant-baseline', 'hanging');
    SvgChartTitle.setAttributeNS(null, 'font-size', '1.3em');
    SvgChartTitle.innerHTML = exchange + ' : ' + security;

    this.SvgCandleRoot.appendChild(SvgChartTitle);

    this.Socket = new SVGServerComs(this.SERVER_IP,
        this.onsocketready.bind(this),
        this.onfullchartreceived.bind(this),
        this.onlatestcandlereceived.bind(this),
        this.onanalysisreceived.bind(this));
  }

  /**
    The full chart data, will intialize the entire svg
    @param {IChart} cht The chart
   */
  private onfullchartreceived(cht: IChart): void {
    // TODO: draw the entire chart

    this.TotalCandlesServerReceived = cht.chart.length;

    let cndIdx: number = 0;
    if (this.TotalCandlesServerReceived >= this.WantedNumberOfCandles) {
      cndIdx = this.TotalCandlesServerReceived - this.WantedNumberOfCandles;
    }
    const idxOffset: number = cndIdx;
    cndIdx = 0;

    // Compute the max and min prices
    this.computePriceTransformations(cht, idxOffset);

    for (cndIdx; cndIdx < cht.chart.length; ++cndIdx) {
      const curCnd: Candle = cht.chart[cndIdx].candle;
      this.CandleDomReferences.push(new SVGCandleStick(this,
          curCnd, cndIdx - idxOffset));
    }

    this.createPriceTicksPlaceHolder();

    if (!this.Socket.getAnalysisData(this.InputExchange, this.InputSecurity)) {
      console.error('unable to get analysis data');
    }
  }

  /**
    Will be called when the candle gets received
    @param {IAnalysis} anl The analysis sent from the server
   */
  private onanalysisreceived(anl: IAnalysis) {
    try {
      // This is the initial analysis received go and update each
      // candle analysis first.
      for (let i: number =
           this.NumberOfCandlePatternsDrawn;
        i < anl.analysis.singleCandle.length; ++i) {
        this.CandleDomReferences[i].putSingleCandleAnalysis(
            anl.analysis.singleCandle[i]);
        this.CandleDomReferences[i].putDoubleCandleAnalysis(
            anl.analysis.doubleCandle[i]);
      }
      this.NumberOfCandlePatternsDrawn = anl.analysis.singleCandle.length - 1;

      for (let i: number =
           this.NumberOfSlopedTrendsDrawn;
        i < anl.analysis.slopedLines.length; ++i) {
        this.CandleDomReferences[anl.analysis.slopedLines[i].e]
            .putSlopedTrendLine(anl.analysis.slopedLines[i]);
      }
      this.NumberOfSlopedTrendsDrawn = anl.analysis.slopedLines.length;

      for (let i: number =
           this.NumberOfHorizontalTrendsDrawn;
        i < anl.analysis.trendLines.length; ++i) {
        this.CandleDomReferences[anl.analysis.trendLines[i].e]
            .putHorizontalTrendLine(anl.analysis.trendLines[i]);
      }
      this.NumberOfHorizontalTrendsDrawn = anl.analysis.trendLines.length;

      if (!this.Socket.getLatestCandle(this.InputExchange,
          this.InputSecurity)) {
        console.error('unable to send latest candle from analysis callback');
      }
    } catch (ex) {
      console.warn('chart is a few milliseconds behind, analysis view delayed' +
                   ' until next candle');
    } finally {
      if (!this.Socket.getLatestCandle(this.InputExchange,
          this.InputSecurity)) {
        console.error('unable to send latest candle from analysis callback');
      }
    }
  }

  /**
    Will update the chart and add candles as needed
    @param {ILatestCandle} cnd The latest candle
   */
  private onlatestcandlereceived(cnd: ILatestCandle): void {
    const latestCandle: Candle = cnd.latestCandle.candle;
    const numReferences: number = this.CandleDomReferences.length;

    this.updatePriceTransformations(latestCandle);
    this.updatePriceTicksValues();

    if (!this.CandleDomReferences[numReferences - 1].tryUpdate(latestCandle)) {
      if (this.TotalCandlesServerReceived >= this.WantedNumberOfCandles) {
        // scoot the candle index of everycandle back 1
        this.CandleDomReferences.forEach((value: SVGCandleStick) => {
          value.shiftLeft();
        });

        // add the new candle
        this.CandleDomReferences.push(new SVGCandleStick(this,
            latestCandle, this.WantedNumberOfCandles - 1));
        this.TotalCandlesServerReceived += 1;
      } else {
        // just add a new candle
        this.CandleDomReferences.push(new SVGCandleStick(this,
            latestCandle, this.TotalCandlesServerReceived));
        this.TotalCandlesServerReceived += 1;
      }

      // since we added a new candle, we will branch off into the analysis
      // callback instead of requesting the latest data
      this.Socket.getAnalysisData(this.InputExchange, this.InputSecurity);
      return;
    }
    this.Socket.getLatestCandle(this.InputExchange, this.InputSecurity);
  }

  /**
    Gets called when the server communication is valid. Will start the live
    update feed
   */
  private onsocketready() {
    if (!this.Socket.getFullChart(this.InputExchange, this.InputSecurity)) {
      console.error('unable to request full chart');
    }
  }

  /**
    Returns the current price transformation function needed to map
    price data to pixel data.
    @return {LinearEquation} The most up to date price transformation function
   */
  public getPriceTransformation(): LinearEquation {
    return this.PriceTransformation;
  }

  /**
    Computes the chart price transofration
    @param {IChart} cht The chart
    @param {number} cndIdx The candle index to start the drawings.
   */
  private computePriceTransformations(cht: IChart, cndIdx: number): void {
    let maxPrice: number = Number.NEGATIVE_INFINITY;
    let minPrice: number = Number.POSITIVE_INFINITY;

    for (let i: number = cndIdx; i < cht.chart.length; ++i) {
      if (cht.chart[i].candle.h > maxPrice) maxPrice = cht.chart[i].candle.h;
      if (cht.chart[i].candle.l < minPrice) minPrice = cht.chart[i].candle.l;
    }

    this.PriceTransformation = new LinearEquation(minPrice, 1080,
        maxPrice, 27.2);
    this.PixelTransformation = new LinearEquation(1080, minPrice,
        27.2, maxPrice);
  }

  /**
    If needed change the range of the chart and the candle to accodimate
    for the new price change
    @param {Candle} cnd The newly added candle
   */
  private updatePriceTransformations(cnd: Candle): void {
    let maxPrice: number = Number.NEGATIVE_INFINITY;
    let minPrice: number = Number.POSITIVE_INFINITY;

    let cndIdx: number = 0;
    if (this.TotalCandlesServerReceived >= this.WantedNumberOfCandles) {
      cndIdx = this.TotalCandlesServerReceived - this.WantedNumberOfCandles;
    }

    for (let i: number = cndIdx; i < this.CandleDomReferences.length; ++i) {
      if (this.CandleDomReferences[i].RawCandle.h > maxPrice) {
        maxPrice = this.CandleDomReferences[i].RawCandle.h;
      }
      if (this.CandleDomReferences[i].RawCandle.l < minPrice) {
        minPrice = this.CandleDomReferences[i].RawCandle.l;
      }
    }
    if (cnd.h > maxPrice) maxPrice = cnd.h;
    if (cnd.l < minPrice) minPrice = cnd.l;

    if (maxPrice != this.PriceTransformation.x2 ||
        minPrice != this.PriceTransformation.x1) {
      this.PriceTransformation = new LinearEquation(minPrice, 1080,
          maxPrice, 27.2);
      this.PixelTransformation = new LinearEquation(1080, minPrice,
          27.2, maxPrice);

      // Loop through each candle and force update

      this.CandleDomReferences.forEach( (value: SVGCandleStick) => {
        value.forceUpdate();
      });
    }
  }

  /**
    Gets the svg root dom reference
    @return {SVGElement} The svg root dom reference
   */
  public getSVGCandlesDom(): SVGElement {
    return this.SvgCandleRoot;
  }

  /**
    Creates the text objects in the svg for the price ticks
   */
  private createPriceTicksPlaceHolder(): void {
    for (let i: number = 1; i < this.ChartStyleNumTicks; ++i) {
      const yoffset: number = i * (1080.0 / this.ChartStyleNumTicks);

      const yAxisText: SVGTextElement =
        <SVGTextElement> document.createElementNS(SVGCandleChart.SVG_NS,
            'text');
      yAxisText.setAttributeNS(null, 'x', '2010');
      yAxisText.setAttributeNS(null, 'y', yoffset.toString());
      yAxisText.setAttributeNS(null, 'font-size', '1.3em');
      yAxisText.setAttributeNS(null, 'text-rendering', 'optimizeLegibility');
      yAxisText.setAttributeNS(null, 'dominant-baseline', 'middle');

      // TODO: CHANGE SERVER CODE TO MAKE THIS DIVISOR SEND THROUGH
      // TODO: THE JSON
      yAxisText.innerHTML =
        (this.PixelTransformation.eval(yoffset)/100000).toFixed(5);
      this.TickPriceDomReferences.push(yAxisText);
      this.SvgYAxisRoot.appendChild(yAxisText);

      const yAxisTextLine: SVGTextElement =
        <SVGTextElement> document.createElementNS(SVGCandleChart.SVG_NS,
            'line');
      yAxisTextLine.setAttributeNS(null, 'x1', '2005');
      yAxisTextLine.setAttributeNS(null, 'x2', '2010');
      yAxisTextLine.setAttributeNS(null, 'y1', yoffset.toString());
      yAxisTextLine.setAttributeNS(null, 'y2', yoffset.toString());
      yAxisTextLine.setAttributeNS(null, 'stroke', 'black');

      this.SvgYAxisRoot.appendChild(yAxisTextLine);
    }
  }

  /**
    TODO
   */
  private updatePriceTicksValues(): void {
    for (let i: number = 1; i < this.ChartStyleNumTicks; ++i) {
      const yoffset: number = i * (1080.0 / this.ChartStyleNumTicks);
      this.TickPriceDomReferences[i-1].innerHTML =
        (this.PixelTransformation.eval(yoffset)/100000).toFixed(5);
    }
  }

  /**
    Returns the candle width in pixels of the currently drawn candles
    @return {number} The width in pixels
   */
  public getCandleWidth(): number {
    return this.CandleWidth;
  }
}

window.onload = () => {
  new SVGCandleChart(0, 'OANDA', 'EUR_USD');
  new SVGCandleChart(1, 'OANDA', 'USD_JPY');
  new SVGCandleChart(2, 'OANDA', 'AUD_CAD');
  new SVGCandleChart(3, 'OANDA', 'USD_CAD');
};
