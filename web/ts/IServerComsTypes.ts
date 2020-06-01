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


interface CandlePattern {
  candlesSpanning: number;
  shortCode: string;
}

interface TrendLine {
  endIndex: number;
  startIndex: number;
  direction: number;
}

interface Analysis {
  type: number;
  data: Array<CandlePattern | TrendLine>;
}

interface IAnalysis {
  fullAnalysis: Array<Analysis | null>;
}
