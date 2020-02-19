interface Candle {
  // open
  o: number;

  // high
  h: number;

  // low
  l: number;

  // close
  c: number;

  // volume
  v: number;

  // start timestamp
  s: any;

  // end timestamp
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
  latestCandle: LatestCandle;
}
