interface Candle {
  o: number;
  h: number;
  l: number;
  c: number;
  v: number;
  s: any;
  e: any;
}

interface Chart {
  candle: Candle;
}

interface RootChartObject {
  chart: Chart[];
}

interface LatestCandle {
  candle: Candle;
}

interface LatestChartCandle {
  latest_candle: LatestCandle;
}
