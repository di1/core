interface TrendLine {
  s: number;
  e: number;
  d: boolean;
}

interface AnalysisJSON {
  singleCandle: number[];
  trendLines: TrendLine[];
}

interface RootAnalysisJSON {
  analysis: AnalysisJSON;
}
