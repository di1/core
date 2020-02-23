interface TrendLine {
  s: number;
  e: number;
  d: number;
}

interface AnalysisJSON {
  singleCandle: number[];
  trendLines: TrendLine[];
}

interface RootAnalysisJSON {
  analysis: AnalysisJSON;
}
