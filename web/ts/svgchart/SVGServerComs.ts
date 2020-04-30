type SocketReadyFunc = () => any;
type FullChartReceivedFunc = (cht: IChart) => any;
type LatestCandleReceivedFunc = (cnd: ILatestCandle) => any;

/**
  An abstraction to websocket that allows for callback based server
  communication
 */
class SVGServerComs { // eslint-disable-line no-unused-vars
  private socket: WebSocket;

  /**
    Function to call when websocket connection is opened
   */
  private onsocketready: SocketReadyFunc;

  /**
    Function to call when server sends a initially chart
   */
  private onfullchartreceived: FullChartReceivedFunc

  /**
    Function to call wehn the server sends the latest candle
   */
  private onlatestcandlereceived: LatestCandleReceivedFunc;

  /**
    Creates a new connection and sets up the bindings.
    @param {string} ip The servers ip address
    @param {SocketReadyFunc} onsocketready Callback when connection is acheived.
    @param {FullChartReceivedFunc} onfullchartreceived Callback when server
    sends full chart information.
    @param {LatestCandleReceivedFunc} onlatestcandlereceived Callback when
    server sends latest candle information
   */
  constructor(ip: string, onsocketready: SocketReadyFunc,
      onfullchartreceived: FullChartReceivedFunc,
      onlatestcandlereceived: LatestCandleReceivedFunc) {
    this.socket = new WebSocket(ip, 'lws-minimal');

    this.onsocketready = onsocketready;
    this.onfullchartreceived = onfullchartreceived;
    this.onlatestcandlereceived = onlatestcandlereceived;

    this.socket.onmessage = this.onmessage.bind(this);
    this.socket.onopen = this.onopen.bind(this);
    this.socket.onclose = this.onclose.bind(this);
    this.socket.onerror = this.onerror.bind(this);
  }

  /**
    Asks the server for the latest chart.
    @param {string} exchange The exchange to pull from
    @param {string} security The ticker/security symbol
    @return {boolean} Returns false if socket isn't open otherwize returns true.
   */
  public getFullChart(exchange: string, security: string): boolean {
    if (this.socket.readyState != this.socket.OPEN) {
      return false;
    } else {
      this.socket.send('init|' + exchange + ':' + security);
      return true;
    }
  }

  /**
    Asks the server for the latest candle.
    @param {string} exchange The exchange to pull from
    @param {string} security The ticker/security symbol
    @return {boolean} Returns false if socket isn't open otherwize returns true.
   */
  public getLatestCandle(exchange: string, security: string): boolean {
    if (this.socket.readyState != this.socket.OPEN) {
      return false;
    } else {
      this.socket.send('latest|' + exchange + ':' + security);
      return true;
    }
  }

  /**
    When a message is received this will get called and the function will
    dispatch the message to the callback designated with the json response.
    @param {MessageEvent} evt The message event
   */
  private onmessage(evt: MessageEvent) {
    /*
      When the message is received, identify the type of message and disburse
      invoke the callback function
     */
    const response: any = JSON.parse(evt.data);
    if (response['chart'] !== undefined) {
      this.onfullchartreceived(<IChart>response);
    } else if (response['latestCandle']) {
      this.onlatestcandlereceived(<ILatestCandle>response);
    }
  }

  /**
    Callback for websocket onopen
    @param {Event} evt The event
   */
  private onopen(evt: Event) {
    console.log('websocket opened');
    this.onsocketready();
  }


  /**
    Callback for websocket onclose
    @param {Event} evt The event
   */
  private onclose(evt: Event) {
    console.log('websocket closed');
  }

  /**
    Callback for websocket onerror
    @param {Event} evt The event
   */
  private onerror(evt: Event) {
    console.error(evt);
  }
}
