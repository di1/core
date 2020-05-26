/**
  Holds the 3 sub charts
 */
class Chart {// eslint-disable-line no-unused-vars
  /**
    Holds the entire chart display and its
    sub elements.
   */
  private Container: HTMLDivElement;

  /**
    The three subsections of the chart
   */
  private CandleChart: HTMLCanvasElement;
  private ChartOptions: HTMLDivElement;
  private ChartOptionsSearchIcon: HTMLObjectElement;
  private ChartOptionsSearchInput: HTMLInputElement;
  private ChartCandleView: ChartCandleView | null = null;

  /**
    Represents the exchange and symbol to pull data from
   */
  private Exchange: string = 'OANDA';
  private Symbol: string = 'EUR_USD';

  /**
    Represents the color scheme
   */
  private Theme: ITheme;

  /**
    Handles communication between the server and
    the charts. Each chart gets their own websocket
    connection.
   */
  private Socket: ServerComs;

  /**
    Default constructor for a chart
    @param {string} exchange The initial exchange (this can be changed)
    @param {string} symbol The initial symbol (this can be changed)
    @param {ITheme} theme The theme to use
   */
  constructor(exchange: string, symbol: string, theme: ITheme) {
    this.Theme = theme;
    this.Exchange = exchange;
    this.Symbol = symbol;

    // Create new container
    this.Container =
      document.createElement('div') as HTMLDivElement;

    this.Container.style.margin = '10px';
    this.Container.style.width = 'calc(100% - 22px)';
    this.Container.style.height = 'calc(100% - 22px)';
    this.Container.style.border = '1px solid ' + this.Theme.ui;

    document.body.appendChild(this.Container);

    // Create the options bar
    this.ChartOptions =
      document.createElement('div') as HTMLDivElement;
    this.ChartOptions.classList.add('chart-options');
    this.ChartOptions.style.float = 'left';
    this.ChartOptions.style.width = 'calc(100%)';
    this.ChartOptions.style.height = '2em';
    this.ChartOptions.style.display = 'flex';
    this.Container.appendChild(this.ChartOptions);

    this.ChartOptionsSearchInput =
      document.createElement('input') as HTMLInputElement;
    this.ChartOptionsSearchInput.style.backgroundColor = this.Theme.bg;
    this.ChartOptionsSearchInput.style.color = this.Theme.fg;
    this.ChartOptionsSearchInput.style.border = 'none';
    this.ChartOptionsSearchInput.style.paddingLeft = '1em';

    this.ChartOptionsSearchInput.value = this.Exchange + ':' +
      this.Symbol;

    this.ChartOptions.appendChild(this.ChartOptionsSearchInput);

    this.ChartOptionsSearchIcon =
      document.createElement('object') as HTMLObjectElement;
    this.ChartOptionsSearchIcon.data = 'img/icons/search-24px.svg';
    this.ChartOptionsSearchIcon.type = 'image/svg+xml';
    this.ChartOptionsSearchIcon.style.borderRight = '1px solid var(--fg)';

    this.ChartOptionsSearchIcon.onload = ((evt: Event) => {
      const svgicon: Document | null =
        (<HTMLObjectElement>evt.srcElement).getSVGDocument();
      console.log(svgicon);
      if (svgicon) {
        (<SVGPathElement>svgicon.querySelector('path'))
            .setAttributeNS(null, 'fill', '#B3B1AD');
      }
    });

    this.ChartOptions.appendChild(this.ChartOptionsSearchIcon);


    // Create the two canvas
    this.CandleChart =
      document.createElement('canvas') as HTMLCanvasElement;
    this.CandleChart.style.float = 'left';
    this.CandleChart.style.width = 'calc(100%)';
    this.CandleChart.style.height = 'calc(100% - 2em)';

    this.Container.appendChild(this.CandleChart);

    // Create the socket connection
    this.Socket = new ServerComs('ws://riski.sh:7681',
        this.onsocketready.bind(this),
        this.onfullchartreceived.bind(this),
        this.onlatestcandlereceived.bind(this),
        this.analysisreceivedfunc.bind(this));
  }

  /**
    Callback for when the socket has opened up and connected
    successfully.
   */
  private onsocketready(): any {
    console.log('Connected to ws://' + document.domain + ':7681');

    // start up the chart candle view
    this.Socket.getFullChart(this.Exchange, this.Symbol);
  }

  /**
    Callback when the server sends full chart data
    @param {IChart} cht The chart interface
   */
  private onfullchartreceived(cht: IChart): void {
    if (this.ChartCandleView) {
      this.ChartCandleView.chartfullUpdate(cht);
    } else {
      this.ChartCandleView = new ChartCandleView(this.CandleChart, cht);
    }
    this.Socket.getLatestCandle(this.Exchange, this.Symbol);
  }

  /**
    Callback when the server sends the latest candle data
    @param {ILatestCandle} cnd The latest candle
   */
  private onlatestcandlereceived(cnd: ILatestCandle): void {
    if (this.ChartCandleView) {
      if (!this.ChartCandleView.chartPartialUpdate(cnd)) {
        this.Socket.getFullChart(this.Exchange, this.Symbol);
      } else {
        this.Socket.getLatestCandle(this.Exchange, this.Symbol);
      }
    }
  }

  /**
    Callback when the server sends analysis data back.
    @param {IAnalysis} anl The analysis data
   */
  private analysisreceivedfunc(anl: IAnalysis): void {

  }
}
