enum ANALYSIS_DATA_TYPE {// eslint-disable-line no-unused-vars
  CANDLE_PATTERN = 0, // eslint-disable-line no-unused-vars
  TREND_LINE = 1// eslint-disable-line no-unused-vars
}

enum TREND_LINE_DIRECTION {// eslint-disable-line no-unused-vars
  SUPPORT = 0, // eslint-disable-line no-unused-vars
  RESISTANCE = 1// eslint-disable-line no-unused-vars
}

interface Candle {
  o: number;
  h: number;
  l: number;
  c: number;
  s: number;
  e: number;
  v: number;
  b: number;
  a: number;
}

interface ICandle {
  candle: Candle;
}

interface Chart {
  precision: number;
  candles: ICandle[];
}

interface IChart {
  chart: Chart;
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
  type: ANALYSIS_DATA_TYPE;
  data: CandlePattern | TrendLine;
}

interface IAnalysis {
  analysisFull: Array<Array<Analysis> | null>;
}
