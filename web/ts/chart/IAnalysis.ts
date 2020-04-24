interface TrendLine {
  s: number;
  e: number;
  d: number;
  score: number;
}

interface AnalysisJSON {
  singleCandle: number[];
  doubleCandle: number[];
  trendLines: TrendLine[];
  slopedLines: TrendLine[];
}

interface RootAnalysisJSON {
  analysis: AnalysisJSON;
}
