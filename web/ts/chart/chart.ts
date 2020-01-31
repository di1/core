class CandleChart extends HTMLCanvasElement {

  private symbol: string;
  private conn: WebSocket;

  constructor(symbol: string) {
    super();
    this.symbol = symbol;
    this.conn = new WebSocket("ws://localhost:7681", "lws-minimal");

    this.conn.onopen = this.connOnOpen;
  }

  connOnOpen() {

  }

}
