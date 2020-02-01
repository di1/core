class CandleChart {

  private symbol: string;
  private conn: WebSocket;

  constructor(symbol: string) {
    this.symbol = symbol;
    this.conn = new WebSocket("ws://localhost:7681", "lws-minimal");
    this.conn.onopen = this.onOpen.bind(this);
    this.conn.onclose = this.onClose.bind(this);
    this.conn.onmessage = this.onMessage.bind(this);
  }

  private onMessage(evt: MessageEvent) {
    console.log(evt.data);
  }

  private onOpen(evt: Event) {
    this.conn.send('init|' + this.symbol);
  }

  private onClose() {

  }

}
