interface TrendLine {
  s: number;
  e: number;
  d: boolean;
}

interface AnalysisJSON {
  single_candle: number[];
  trend_lines: TrendLine[];
}

interface RootAnalysisJSON {
  analysis: AnalysisJSON;
}
