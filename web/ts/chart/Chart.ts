/// <reference path="IChart.ts" />

class CandleChart {

  private symbol: string;
  private conn: WebSocket;
  private chart_canvas: HTMLCanvasElement | null;
  
  constructor(symbol: string) {
    this.chart_canvas = <HTMLCanvasElement> document.getElementById("chart");

    if (!this.chart_canvas) {
      console.error("chart does not exist in html");
    }

    this.symbol = symbol;
    this.conn = new WebSocket("ws://localhost:7681", "lws-minimal");
    this.conn.onopen = this.onOpen.bind(this);
    this.conn.onclose = this.onClose.bind(this);
    this.conn.onmessage = this.onMessage.bind(this);
  
    this.rescaleCanvas(this.chart_canvas);
  
  }

  private onMessage(evt: MessageEvent) {
    let latestChart: RootChartObject | undefined = JSON.parse(evt.data);

    if (!latestChart) {
      console.error('onMessage latestChart == undefined');
      return;
    }

    this.drawFull(latestChart);
  }

  private onOpen(evt: Event) {
    this.conn.send('init|' + this.symbol);
  }

  private onClose() {
    console.log('websocket connection closed');
  }

  public rescaleCanvas(canvas: any): void {
    // finally query the various pixel ratios
    let ctx = canvas.getContext('2d');

    let devicePixelRatio = window.devicePixelRatio || 1;

    let backingStoreRatio = ctx.webkitBackingStorePixelRatio ||
                        ctx.mozBackingStorePixelRatio ||
                        ctx.msBackingStorePixelRatio ||
                        ctx.oBackingStorePixelRatio ||
                        ctx.backingStorePixelRatio || 1;
    let ratio = devicePixelRatio / backingStoreRatio;

    // upscale the canvas if the two ratios don't match
    if (devicePixelRatio !== backingStoreRatio) {

      let oldWidth = canvas.width;
      let oldHeight = canvas.height;

      canvas.width = oldWidth * ratio;
      canvas.height = oldHeight * ratio;

      canvas.style.width = oldWidth + 'px';
      canvas.style.height = oldHeight + 'px';

      // now scale the context to counter
      // the fact that we've manually scaled
      // our canvas element
      ctx.scale(ratio, ratio);
    }
  }

  private drawFull(chart: RootChartObject) {
    if (!this.chart_canvas) {
      console.error('unable to get canvas element');
      return;
    }

    let ctx: CanvasRenderingContext2D | null = this.chart_canvas.getContext('2d'); 
    let drawing_width: number = this.chart_canvas.width;
    let drawing_height: number = this.chart_canvas.height;

    if (!ctx) {
      console.error('contex == undefined');
      return;
    }

    ctx.strokeStyle = 'black';
 
    let candles: Chart[] = chart.chart;

    for (let i: number = 0; i < candles.length; ++i) {
      console.log(candles[i]);
    }

  }

}
