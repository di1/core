interface Candle {
    o: number;
    h: number;
    l: number;
    c: number;
    s: any;
    e: any;
}

interface Chart {
    candle: Candle;
}

interface RootChartObject {
    chart: Chart[];
}
