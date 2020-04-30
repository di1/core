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
  private RawCandle: Candle;

  /**
    The candle index 0 being the first candle
   */
  private cndIdx: number;

  /**
    The root chart this candle belongs to
   */
  private rootChart: SVGCandleChart;

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


    cb.setAttributeNS(null, 'width', '10px');
    // Note the this.o and c this.c are inverted after the transformation.
    // this.o real value gets smaller as it gets better. So the lowest price
    // would be the biggest number and visaversa
    if (this.PriceTransformedCandle.o > this.PriceTransformedCandle.c) {
      cb.setAttributeNS(null, 'height',
          (this.PriceTransformedCandle.o - this.PriceTransformedCandle.c)
              .toString() + 'px');
      cb.setAttributeNS(null, 'y', this.PriceTransformedCandle.c.toString() +
                        'px');
      cb.setAttributeNS(null, 'fill', 'green');
    } else if (this.PriceTransformedCandle.o < this.PriceTransformedCandle.c) {
      cb.setAttributeNS(null, 'height',
          (this.PriceTransformedCandle.c - this.PriceTransformedCandle.o).
              toString() + 'px');
      cb.setAttributeNS(null, 'y', this.PriceTransformedCandle.o.toString() +
                        'px');
      cb.setAttributeNS(null, 'fill', 'red');
    }

    cb.setAttributeNS(null, 'x', (this.cndIdx * 10).toString() + 'px');

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

    cw.setAttributeNS(null, 'x1', ((this.cndIdx * 10) + 5).toString() + 'px');
    cw.setAttributeNS(null, 'y1', this.PriceTransformedCandle.h.toString());
    cw.setAttributeNS(null, 'x2', ((this.cndIdx * 10) + 5).toString() + 'px');
    cw.setAttributeNS(null, 'y2', this.PriceTransformedCandle.l.toString());
    cw.setAttributeNS(null, 'stroke', 'black');

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
}
