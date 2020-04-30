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
    Stores all the candle dom references in memory to avoid
    excessive dom quriese with document.get or document.select
   */
  private CandleDomReferences: Array<SVGCandleStick> = [];

  /**
    Creates a new live svg chart
   */
  constructor() {
    // Gets the root chart from the body
    this.SvgCandleRoot =
      <SVGElement> document.getElementsByTagNameNS(SVGCandleChart.SVG_NS,
          'g')[0];

    this.SvgYAxisRoot =
      <SVGElement> document.getElementsByTagNameNS(SVGCandleChart.SVG_NS,
          'g')[1];

    this.Socket = new SVGServerComs(this.SERVER_IP,
        this.onsocketready.bind(this),
        this.onfullchartreceived.bind(this),
        this.onlatestcandlereceived.bind(this));
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

    this.Socket.getLatestCandle('OANDA', 'EUR_USD');
  }

  /**
    Will update the chart and add candles as needed
    @param {ILatestCandle} cnd The latest candle
   */
  private onlatestcandlereceived(cnd: ILatestCandle): void {
    const latestCandle: Candle = cnd.latestCandle.candle;
    const numReferences: number = this.CandleDomReferences.length;

    if (!this.CandleDomReferences[numReferences - 1].tryUpdate(latestCandle)) {
      if (this.TotalCandlesServerReceived >= this.WantedNumberOfCandles) {
        // scoot the candle index of everycandle back 1
        this.CandleDomReferences.forEach((value: SVGCandleStick) => {
          value.shiftLeft();
        });

        this.CandleDomReferences.push(new SVGCandleStick(this,
            latestCandle, this.WantedNumberOfCandles - 1));
        this.TotalCandlesServerReceived += 1;
      } else {
        this.CandleDomReferences.push(new SVGCandleStick(this,
            latestCandle, this.TotalCandlesServerReceived - 1));
        this.TotalCandlesServerReceived += 1;
      }
    }

    setTimeout(() => {
      this.Socket.getLatestCandle('OANDA', 'EUR_USD');
    }, 0);
  }

  /**
    Gets called when the server communication is valid. Will start the live
    update feed
   */
  private onsocketready() {
    if (!this.Socket.getFullChart('OANDA', 'EUR_USD')) {
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
        maxPrice, 0);
    this.PixelTransformation = new LinearEquation(1080, minPrice,
        0, maxPrice);
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

      // TODO: CHANGE SERVER CODE TO MAKE THIS DIVISOR SEND THROUGH
      // TODO: THE JSON
      yAxisText.innerHTML =
        (this.PixelTransformation.eval(yoffset)/100000).toFixed(5);
      this.SvgYAxisRoot.appendChild(yAxisText);
    }
  }

  /**
    TODO
   */
  private UpdatePriceTicksValues(): void {

  }
}

window.onload = () => {
  new SVGCandleChart();
};
