/**
  An abstraction to svg representing a candle stick object
 */
class SVGCandleStick { // eslint-disable-line no-unused-vars
  /**
    The representation of the candle body as an svg rect element
   */
  private candleBody: SVGRectElement | null = null;

  /**
    The representation of a candle wick as an svg line element
   */
  private candleWick: SVGLineElement | null = null;

  /**
    A group that holds the candleBody and candle wick
   */
  private candleGroup: SVGGElement;

  /**
    This candle is the transformed prices
   */
  private PriceTransformedCandle: Candle;

  /**
    This candle contains the raw prices
   */
  public RawCandle: Candle;

  /**
    The candle index 0 being the first candle
   */
  private cndIdx: number;

  /**
    The root chart this candle belongs to
   */
  private rootChart: SVGCandleChart;

  /**
    Lookup table defining the single candle lookups
   */
  private SingleCandleLookUp: {[key: string]: any} = {
    1: 'M',
    2: 'M',
    3: 'S',
    4: 'S',
    5: 'D',
    6: 'D',
    7: 'D',
  }

  /**
    Creates the initial candle body. The candle body is represented as an svg
    rect object.
    @return {SVGRectElement | null} Returns null if the candle was updated
    otherwize returns a reference to the newly created candle.
   */
  private buildCandleBody(): SVGRectElement | null {
    // Create the rect element
    let cb: SVGRectElement;
    let createdNewCandle: boolean = false;
    if (this.candleBody === null) {
      cb = <SVGRectElement> document.createElementNS(
          SVGCandleChart.SVG_NS, 'rect');
      createdNewCandle = true;
    } else {
      cb = this.candleBody;
    }


    cb.setAttributeNS(null, 'width',
        this.rootChart.getCandleWidth().toString());
    // Note the this.o and c this.c are inverted after the transformation.
    // this.o real value gets smaller as it gets better. So the lowest price
    // would be the biggest number and visaversa

    // If this is a doji there are some visual that can't be shows using
    // crispEdges
    let doji: boolean = false;

    if (this.PriceTransformedCandle.o > this.PriceTransformedCandle.c) {
      cb.setAttributeNS(null, 'height',
          (this.PriceTransformedCandle.o - this.PriceTransformedCandle.c)
              .toString() + 'px');
      cb.setAttributeNS(null, 'y', this.PriceTransformedCandle.c.toString() +
                        'px');
      cb.setAttributeNS(null, 'fill', '#228B22');
    } else if (this.PriceTransformedCandle.o < this.PriceTransformedCandle.c) {
      cb.setAttributeNS(null, 'height',
          (this.PriceTransformedCandle.c - this.PriceTransformedCandle.o).
              toString() + 'px');
      cb.setAttributeNS(null, 'y', this.PriceTransformedCandle.o.toString() +
                        'px');
      cb.setAttributeNS(null, 'fill', '#E62020');
    } else {
      cb.setAttributeNS(null, 'height', '1');
      cb.setAttributeNS(null, 'y', this.PriceTransformedCandle.o.toString() +
                        'px');
      doji = true;
    }

    cb.setAttributeNS(null, 'x', (this.cndIdx *
                          this.rootChart.getCandleWidth()).toString() + 'px');
    cb.setAttributeNS(null, 'shape-rendering', 'geometricPrecision');

    if (createdNewCandle) {
      return cb;
    } else {
      return null;
    }
  }

  /**
    Creates the initial candle wick. The candle wick is represented as an svg
    rect object.
    @return {SVGLineElement | null} Returns null if the candle wick was updated
    otherwize returns a reference to the newly created candle wick.
   */
  private buildCandleWick(): SVGLineElement | null {
    let cw: SVGLineElement;
    let createdNewWick: boolean = false;

    if (this.candleWick === null) {
      cw = <SVGLineElement> document.createElementNS(SVGCandleChart.SVG_NS,
          'line');
      createdNewWick = true;
    } else {
      cw = this.candleWick;
    }

    cw.setAttributeNS(null, 'x1',
        ((this.cndIdx *
                   this.rootChart.getCandleWidth()) +
                   this.rootChart.getCandleWidth() / 2.0).toString() + 'px');
    cw.setAttributeNS(null, 'y1', this.PriceTransformedCandle.h.toString());
    cw.setAttributeNS(null, 'x2', ((this.cndIdx *
                                   this.rootChart.getCandleWidth()) +
                                   this.rootChart.getCandleWidth() / 2.0)
        .toString() + 'px');
    cw.setAttributeNS(null, 'y2', this.PriceTransformedCandle.l.toString());
    cw.setAttributeNS(null, 'stroke-width', '1');
    cw.setAttributeNS(null, 'stroke', 'black');
    cw.setAttributeNS(null, 'shape-rendering', 'geometricPrecision');

    if (createdNewWick) {
      return cw;
    } else {
      return null;
    }
  }

  /**
    Creates a new candle and adds it to the DOM
    @param {SVGCandleChart} cht The root chart object
    @param {Candle} cnd Initial candle data
    @param {number} cndIdx The initial candle index
   */
  constructor(cht: SVGCandleChart, cnd: Candle, cndIdx: number) {
    this.PriceTransformedCandle = cnd;
    this.rootChart = cht;
    this.RawCandle = <Candle>{'o': cnd.o, 'h': cnd.h, 'l': cnd.l, 'c': cnd.c};
    this.performCandleTransformation();

    this.cndIdx = cndIdx;
    this.candleBody = this.buildCandleBody();
    this.candleWick = this.buildCandleWick();

    this.candleGroup =
      <SVGGElement>document.createElementNS(SVGCandleChart.SVG_NS, 'g');

    if (this.candleWick) {
      this.candleGroup.appendChild(this.candleWick);
    } else {
      console.error('this.candleWick === null');
    }

    if (this.candleBody) {
      this.candleGroup.appendChild(this.candleBody);
    } else {
      console.error('this.candleBody === null');
    }

    // Adds the candle to the svg
    cht.getSVGCandlesDom().appendChild(this.candleGroup);
  }

  /**
    If the given latest candle is data for this objects candle it will update
    iteslef and subsiquent changes will happen automatically on the DOM
    @param {Candle} latestCandle The latest candle data
    @return {boolean} True if a new candle doesn't need to be created because
    this reference is to old
   */
  public tryUpdate(latestCandle: Candle): boolean {
    if (latestCandle.s === this.PriceTransformedCandle.s) {
      // Don't update the dom if the data hasen't changed
      if (latestCandle.o === this.RawCandle.o &&
          latestCandle.h === this.RawCandle.h &&
          latestCandle.l === this.RawCandle.l &&
          latestCandle.c === this.RawCandle.c) {
        return true;
      }
      this.PriceTransformedCandle = latestCandle;
      this.RawCandle = <Candle>{'o': latestCandle.o, 'h': latestCandle.h,
        'l': latestCandle.l, 'c': latestCandle.c};
      this.performCandleTransformation();
      this.buildCandleBody();
      this.buildCandleWick();
      return true;
    } else {
      return false;
    }
  }

  /**
    Converts candle OHLC into pixel coordinates
   */
  private performCandleTransformation() {
    this.PriceTransformedCandle.o =
      this.rootChart.getPriceTransformation().eval(
          this.PriceTransformedCandle.o);
    this.PriceTransformedCandle.h =
      this.rootChart.getPriceTransformation().eval(
          this.PriceTransformedCandle.h);
    this.PriceTransformedCandle.l =
      this.rootChart.getPriceTransformation().eval(
          this.PriceTransformedCandle.l);
    this.PriceTransformedCandle.c =
      this.rootChart.getPriceTransformation().eval(
          this.PriceTransformedCandle.c);
  }

  /**
    Shifts the candle 1 candle to the left to make room for other candles on the
    right
   */
  public shiftLeft() {
    this.cndIdx -= 1;
    this.buildCandleBody();
    this.buildCandleWick();
  }

  /**
    Adds the single candle analysis text object to this candle
    group.
    @param {number} id The ID specifiying what type of single candle pattern
    this candle represents.
   */
  public putSingleCandleAnalysis(id: number) {
    switch (id) {
      case 0:
        break;
      default:
        const singleCandleDisplay: SVGTextElement =
          <SVGTextElement> document.createElementNS(SVGCandleChart.SVG_NS,
              'text');
        singleCandleDisplay.setAttributeNS(null, 'x',
            (this.cndIdx * this.rootChart.getCandleWidth()).toString());
        singleCandleDisplay.setAttributeNS(null, 'y',
            (this.PriceTransformedCandle.l + 1).toString());
        singleCandleDisplay.setAttributeNS(null, 'font-size',
            (this.rootChart.getCandleWidth()*(1.5)).toString() + 'px');
        singleCandleDisplay.setAttributeNS(null, 'dominant-baseline',
            'hanging');
        singleCandleDisplay.innerHTML = this.SingleCandleLookUp[id];

        this.candleGroup.appendChild(singleCandleDisplay);
    }
  }

  public putDoubleCandleAnalysis(id: number) {
    switch (id) {
      case 0:
        break;
      default:
        console.log('abc');
        const doubleCandleDisplay: SVGRectElement =
          <SVGRectElement> document.createElementNS(SVGCandleChart.SVG_NS,
                                                    'rect');
        doubleCandleDisplay.setAttributeNS(null, 'x',
           ((this.cndIdx * this.rootChart.getCandleWidth()) -
             (this.rootChart.getCandleWidth())).toString());
        doubleCandleDisplay.setAttributeNS(null, 'y',
                                 this.PriceTransformedCandle.h.toString());
        doubleCandleDisplay.setAttributeNS(null, 'width',
            (this.rootChart.getCandleWidth() * 2).toString());
        doubleCandleDisplay.setAttributeNS(null, 'height',
            (this.PriceTransformedCandle.l - this.PriceTransformedCandle.h)
            .toString());
        doubleCandleDisplay.setAttributeNS(null, 'stroke-width', '1');
        doubleCandleDisplay.setAttributeNS(null, 'stroke', 'black');
        doubleCandleDisplay.setAttributeNS(null, 'shape-rendering',
                                           'geometricPrecision');
        doubleCandleDisplay.setAttributeNS(null, 'fill', 'blue');
        doubleCandleDisplay.setAttributeNS(null, 'fill-opacity', '0.3');

        this.candleGroup.appendChild(doubleCandleDisplay);
    }
  }
}
