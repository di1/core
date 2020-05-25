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
  private ChartCandleView: ChartCandleView | null = null;

  /**
    Handles communication between the server and
    the charts. Each chart gets their own websocket
    connection.
   */
  private Socket: ServerComs;

  /**
    Default constructor for a chart
   */
  constructor() {
    // Create new container
    this.Container =
      document.createElement('div') as HTMLDivElement;
    this.Container.classList.add('cht-container');
    document.body.appendChild(this.Container);

    // Create the options bar
    this.ChartOptions =
      document.createElement('div') as HTMLDivElement;
    this.ChartOptions.classList.add('chart-options');
    this.Container.appendChild(this.ChartOptions);

    // Create the two canvas
    this.CandleChart =
      document.createElement('canvas') as HTMLCanvasElement;
    this.CandleChart.classList.add('candle-chart');

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
    this.Socket.getFullChart('OANDA', 'USD_JPY');
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
    this.Socket.getLatestCandle('OANDA', 'USD_JPY');
  }

  /**
    Callback when the server sends the latest candle data
    @param {ILatestCandle} cnd The latest candle
   */
  private onlatestcandlereceived(cnd: ILatestCandle): void {
    if (this.ChartCandleView) {
      if (!this.ChartCandleView.chartPartialUpdate(cnd)) {
        this.Socket.getFullChart('OANDA', 'USD_JPY');
      } else {
        this.Socket.getLatestCandle('OANDA', 'USD_JPY');
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
