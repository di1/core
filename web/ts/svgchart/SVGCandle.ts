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
  public PriceTransformedCandle: Candle;

  /**
    This candle contains the raw prices
   */
  public RawCandle: Candle;

  /**
    The candle index 0 being the first candle
    These needs to be public for other candles to know where this candle is
   */
  public CndIdx: number;

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
    Holds the text element saying what kind of single candle analysis
    we have. (dom reference)
   */
  private SingleCandleText: SVGTextElement | null = null;

  /**
    Holds the text element id for reproducing the text box
   */
  private SingleCandleTextCode: number | null = null;

  /**
    Holds the line showing the sloped trend (dom reference)
   */
  private SlopedTrendLine: SVGLineElement | null = null;

  /**
    Holds the horizontal trend line (dom reference)
   */
  private HorizontalTrendLine: SVGLineElement | null = null;

  /**
    Holds the information to rebuild the sloped trend line
   */
  private SlopedTrendLineData: Trend | null = null;

  /**
    Holds the information to rebuild the horizontal trend line
   */
  private HorizontalTrendLineData: Trend | null = null;

  /**
    The shaded are for double candle analysis (dom reference)
   */
  private DoubleCandleAnalysis: SVGRectElement | null = null;

  /**
    The double candle analysis code for reproducing the shaded area
   */
  private DoubleCandleAnalysisCode: number | null = null;

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
    }

    cb.setAttributeNS(null, 'x', (this.CndIdx *
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
        ((this.CndIdx *
                   this.rootChart.getCandleWidth()) +
                   this.rootChart.getCandleWidth() / 2.0).toString() + 'px');
    cw.setAttributeNS(null, 'y1', this.PriceTransformedCandle.h.toString());
    cw.setAttributeNS(null, 'x2', ((this.CndIdx *
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
    @param {number} CndIdx The initial candle index
   */
  constructor(cht: SVGCandleChart, cnd: Candle, CndIdx: number) {
    this.PriceTransformedCandle = cnd;
    this.rootChart = cht;
    this.RawCandle = <Candle>{'o': cnd.o, 'h': cnd.h, 'l': cnd.l, 'c': cnd.c};
    this.performCandleTransformation();

    this.CndIdx = CndIdx;
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
  private performCandleTransformation(): void {
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
  public shiftLeft(): void {
    this.CndIdx -= 1;
    this.forceUpdate();
  }

  /**
    Adds the single candle analysis text object to this candle
    group.
    @param {number} id The ID specifiying what type of single candle pattern
    this candle represents.
   */
  public putSingleCandleAnalysis(id: number): void {
    let newlyCreatedCandle: boolean = true;
    switch (id) {
      case 0:
        break;
      default:
        this.SingleCandleTextCode = id;
        if (this.SingleCandleText === null) {
          this.SingleCandleText =
          <SVGTextElement> document.createElementNS(SVGCandleChart.SVG_NS,
              'text');
        } else {
          newlyCreatedCandle = false;
        }
        this.SingleCandleText.setAttributeNS(null, 'x',
            (this.CndIdx * this.rootChart.getCandleWidth()).toString());
        this.SingleCandleText.setAttributeNS(null, 'y',
            (this.PriceTransformedCandle.l + 1).toString());
        this.SingleCandleText.setAttributeNS(null, 'font-size',
            (this.rootChart.getCandleWidth()*1.6).toString() + 'px');
        this.SingleCandleText.setAttributeNS(null, 'dominant-baseline',
            'hanging');
        this.SingleCandleText.setAttributeNS(null, 'font-family', 'monospace');
        this.SingleCandleText.setAttributeNS(null, 'text-anchore', 'middle');
        this.SingleCandleText.innerHTML = this.SingleCandleLookUp[id];

        if (newlyCreatedCandle) {
          this.candleGroup.appendChild(this.SingleCandleText);
        }
    }
  }

  /**
    Adds double candle analysis to the chart by shading the tow candles
    region blue
    @param {number} id
   */
  public putDoubleCandleAnalysis(id: number): void {
    let newlyCreatedCandle: boolean = true;
    switch (id) {
      case 0:
        break;
      default:
        this.DoubleCandleAnalysisCode = id;
        if (this.DoubleCandleAnalysis === null) {
          this.DoubleCandleAnalysis =
            <SVGRectElement> document.createElementNS(SVGCandleChart.SVG_NS,
                'rect');
        } else {
          newlyCreatedCandle = false;
        }
        this.DoubleCandleAnalysis.setAttributeNS(null, 'x',
            ((this.CndIdx * this.rootChart.getCandleWidth()) -
             (this.rootChart.getCandleWidth())).toString());
        this.DoubleCandleAnalysis.setAttributeNS(null, 'y',
            this.PriceTransformedCandle.h.toString());
        this.DoubleCandleAnalysis.setAttributeNS(null, 'width',
            (this.rootChart.getCandleWidth() * 2).toString());
        this.DoubleCandleAnalysis.setAttributeNS(null, 'height',
            (this.PriceTransformedCandle.l - this.PriceTransformedCandle.h)
                .toString());
        this.DoubleCandleAnalysis.setAttributeNS(null, 'stroke-width', '1');
        this.DoubleCandleAnalysis.setAttributeNS(null, 'stroke', 'black');
        this.DoubleCandleAnalysis.setAttributeNS(null, 'shape-rendering',
            'geometricPrecision');
        this.DoubleCandleAnalysis.setAttributeNS(null, 'fill', 'blue');
        this.DoubleCandleAnalysis.setAttributeNS(null, 'fill-opacity', '0.3');

        if (newlyCreatedCandle) {
          this.candleGroup.appendChild(this.DoubleCandleAnalysis);
        }
    }
  }

  /**
    Puts a sloped trend line on the chart
    @param {Trend} tnd The trend line to put
   */
  public putSlopedTrendLine(tnd: Trend): void {
    // Puts a support or ristance trend line on the chart
    this.SlopedTrendLineData = tnd;
    let newlyCreatedCandle: boolean = true;
    switch (tnd.d) {
      case 0: // support
      case 2: // support
        if (this.SlopedTrendLine === null) {
          this.SlopedTrendLine =
            <SVGLineElement> document.createElementNS(SVGCandleChart.SVG_NS,
                'line');
        } else {
          newlyCreatedCandle = false;
        }
        this.SlopedTrendLine.setAttributeNS(null, 'x1',
            ((this.CndIdx * this.rootChart.getCandleWidth()) +
                                   this.rootChart.getCandleWidth() / 2.0)
                .toString());
        this.SlopedTrendLine.setAttributeNS(null, 'y1',
            this.PriceTransformedCandle.l.toString());

        this.SlopedTrendLine.setAttributeNS(null, 'x2',
            ((this.rootChart.CandleDomReferences[tnd.s].CndIdx *
              this.rootChart.getCandleWidth()) +
                this.rootChart.getCandleWidth() / 2.0)
                .toString());
        this.SlopedTrendLine.setAttributeNS(null, 'y2',
            this.rootChart.CandleDomReferences[tnd.s].
                PriceTransformedCandle.l.toString());
        this.SlopedTrendLine.setAttributeNS(null, 'shape-rendering',
            'geometricPrecision');
        this.SlopedTrendLine.setAttributeNS(null, 'stroke-width', '2');
        this.SlopedTrendLine.setAttributeNS(null, 'stroke', '#FE4EDA');

        if (newlyCreatedCandle) {
          this.candleGroup.appendChild(this.SlopedTrendLine);
        }

        break;
      case 1: // resistance
      case 3: // resistance
        if (this.SlopedTrendLine === null) {
          this.SlopedTrendLine =
            <SVGLineElement> document.createElementNS(SVGCandleChart.SVG_NS,
                'line');
        } else {
          newlyCreatedCandle = false;
        }
        this.SlopedTrendLine.setAttributeNS(null, 'x1',
            ((this.CndIdx * this.rootChart.getCandleWidth()) +
                                   this.rootChart.getCandleWidth() / 2.0)
                .toString());
        this.SlopedTrendLine.setAttributeNS(null, 'y1',
            this.PriceTransformedCandle.h.toString());

        this.SlopedTrendLine.setAttributeNS(null, 'x2',
            ((
              this.rootChart.CandleDomReferences[tnd.s].CndIdx *
                this.rootChart.getCandleWidth()) +
                                   this.rootChart.getCandleWidth() / 2.0)
                .toString());
        this.SlopedTrendLine.setAttributeNS(null, 'y2',
            this.rootChart.CandleDomReferences[tnd.s].
                PriceTransformedCandle.h.toString());
        this.SlopedTrendLine.setAttributeNS(null, 'shape-rendering',
            'geometricPrecision');
        this.SlopedTrendLine.setAttributeNS(null, 'stroke-width', '2');
        this.SlopedTrendLine.setAttributeNS(null, 'stroke', '#FE4EDA');

        if (newlyCreatedCandle) {
          this.candleGroup.appendChild(this.SlopedTrendLine);
        }
        break;
    }
  }

  /**
    Puts a horizontal trend line on the chart
    @param {Trend} tnd The trend
   */
  public putHorizontalTrendLine(tnd: Trend): void {
    this.HorizontalTrendLineData = tnd;
    let newlyCreatedCandle: boolean = true;
    switch (tnd.d) {
      case 0: // support
      case 2: // support
        if (this.HorizontalTrendLine == null) {
          this.HorizontalTrendLine =
            <SVGLineElement> document.createElementNS(SVGCandleChart.SVG_NS,
                'line');
        } else {
          newlyCreatedCandle = false;
        }
        this.HorizontalTrendLine.setAttributeNS(null, 'x1',
            (this.CndIdx * this.rootChart.getCandleWidth() +
                this.rootChart.getCandleWidth() / 2.0).toString());
        this.HorizontalTrendLine.setAttributeNS(null, 'y1',
            this.PriceTransformedCandle.l
                .toString());
        this.HorizontalTrendLine.setAttributeNS(null, 'x2',
            (this.rootChart.CandleDomReferences[tnd.s].CndIdx *
                this.rootChart.getCandleWidth() +
                this.rootChart.getCandleWidth() / 2.0).toString());
        this.HorizontalTrendLine.setAttributeNS(null, 'y2',
            this.PriceTransformedCandle.l
                .toString());
        this.HorizontalTrendLine.setAttributeNS(null, 'shape-rendering',
            'geometricPrecision');
        this.HorizontalTrendLine.setAttributeNS(null, 'stroke-width', '1');
        this.HorizontalTrendLine.setAttributeNS(null, 'stroke', 'purple');

        if (newlyCreatedCandle) {
          this.candleGroup.appendChild(this.HorizontalTrendLine);
        }
        break;
      case 1: // resistance
      case 3: // resistance
        if (this.HorizontalTrendLine === null) {
          this.HorizontalTrendLine =
            <SVGLineElement> document.createElementNS(SVGCandleChart.SVG_NS,
                'line');
        } else {
          newlyCreatedCandle = false;
        }
        this.HorizontalTrendLine.setAttributeNS(null, 'x1',
            (this.CndIdx * this.rootChart.getCandleWidth() +
             this.rootChart.getCandleWidth() / 2.0).toString());
        this.HorizontalTrendLine.setAttributeNS(null, 'y1',
            this.PriceTransformedCandle.h
                .toString());
        this.HorizontalTrendLine.setAttributeNS(null, 'x2',
            (this.rootChart.CandleDomReferences[tnd.s].CndIdx *
             this.rootChart.getCandleWidth() +
             this.rootChart.getCandleWidth() / 2.0).toString());
        this.HorizontalTrendLine.setAttributeNS(null, 'y2',
            this.PriceTransformedCandle.h
                .toString());
        this.HorizontalTrendLine.setAttributeNS(null, 'shape-rendering',
            'geometricPrecision');
        this.HorizontalTrendLine.setAttributeNS(null, 'stroke-width', '1');
        this.HorizontalTrendLine.setAttributeNS(null, 'stroke', 'black');

        if (newlyCreatedCandle) {
          this.candleGroup.appendChild(this.HorizontalTrendLine);
        }
        break;
    }
  }


  /**
    Forces an update to the candle body and wick
    TODO: Force update needs to hande the analysis that will be changed
   */
  public forceUpdate() {
    this.PriceTransformedCandle.o = this.RawCandle.o;
    this.PriceTransformedCandle.h = this.RawCandle.h;
    this.PriceTransformedCandle.l = this.RawCandle.l;
    this.PriceTransformedCandle.c = this.RawCandle.c;
    this.performCandleTransformation();
    if (this.CndIdx >= -1) {
      this.buildCandleBody();
      this.buildCandleWick();

      if (this.SlopedTrendLineData && this.SlopedTrendLine) {
        this.putSlopedTrendLine(this.SlopedTrendLineData);
      }

      if (this.HorizontalTrendLineData && this.HorizontalTrendLine) {
        this.putHorizontalTrendLine(this.HorizontalTrendLineData);
      }

      if (this.SingleCandleTextCode && this.SingleCandleText) {
        this.putSingleCandleAnalysis(this.SingleCandleTextCode);
      }

      if (this.DoubleCandleAnalysis && this.DoubleCandleAnalysisCode) {
        this.putDoubleCandleAnalysis(this.DoubleCandleAnalysisCode);
      }
    }
  }
}
