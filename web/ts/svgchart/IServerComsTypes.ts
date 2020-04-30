interface Candle {
  o: number;
  h: number;
  l: number;
  c: number;
  s: number;
  e: number;
  v: number;

}

interface ICandle {
  candle: Candle;
}

interface IChart {
  chart: ICandle[];
}

interface ILatestCandle {
  latestCandle: ICandle;
}
