"use strict";
/// <reference path="IChart.ts" />
/// <reference path="ILinearEquation.ts" />
var CandleChart = /** @class */ (function () {
    function CandleChart(symbol) {
        this.NUM_TICKS = 20;
        this.PADDING_BOT = 15;
        this.PADDING_TOP = 15;
        this.CANDLE_WIDTH = 10;
        this.ROOT_CHART = undefined;
        this.CANDLE_UPDATES_SENT = 0;
        this.CANDLE_SPACING = 5;
        this.MOUSE_X = 0;
        this.MOUSE_Y = 0;
        this.chart_canvas = document.getElementById("chart");
        if (!this.chart_canvas) {
            console.error("chart does not exist in html");
        }
        this.symbol = symbol;
        this.conn = new WebSocket("ws://localhost:7681", "lws-minimal");
        this.conn.onopen = this.onOpen.bind(this);
        this.conn.onclose = this.onClose.bind(this);
        this.conn.onmessage = this.onMessage.bind(this);
        //this.rescaleCanvas(this.chart_canvas);
        this.chart_canvas.onwheel = this.onMouseWheelEvent.bind(this);
        this.chart_canvas.onmousemove = this.onMouseMove.bind(this);
    }
    CandleChart.prototype.onMessage = function (evt) {
        var genericMsg = JSON.parse(evt.data);
        if (genericMsg['chart']) {
            var latestChart = genericMsg;
            if (!latestChart) {
                console.error('onMessage latestChart == undefined');
                return;
            }
            this.ROOT_CHART = latestChart;
            this.drawFull(this.ROOT_CHART);
        }
        else if (genericMsg['latest_candle']) {
            var updateCandle = genericMsg;
            if (!updateCandle) {
                console.error('onMessage updateCandle == undefined');
                return;
            }
            if (!this.ROOT_CHART) {
                console.error('onMessage updateCandle happened before init');
                return;
            }
            if (this.ROOT_CHART.chart[this.ROOT_CHART.chart.length - 1].candle.s ==
                updateCandle.latest_candle.candle.s) {
                this.ROOT_CHART.chart[this.ROOT_CHART.chart.length - 1].candle =
                    updateCandle.latest_candle.candle;
                this.drawFull(this.ROOT_CHART);
            }
            else {
                this.ROOT_CHART.chart.push({ candle: updateCandle.latest_candle.candle });
                this.drawFull(this.ROOT_CHART);
            }
        }
        else {
            console.error('invalid response from server');
        }
    };
    CandleChart.prototype.onOpen = function (evt) {
        this.conn.send('init|' + this.symbol);
    };
    CandleChart.prototype.onClose = function () {
        console.log('websocket connection closed');
    };
    CandleChart.prototype.rescaleCanvas = function (canvas) {
        // finally query the various pixel ratios
        var ctx = canvas.getContext('2d');
        var devicePixelRatio = window.devicePixelRatio || 1;
        var backingStoreRatio = ctx.webkitBackingStorePixelRatio ||
            ctx.mozBackingStorePixelRatio ||
            ctx.msBackingStorePixelRatio ||
            ctx.oBackingStorePixelRatio ||
            ctx.backingStorePixelRatio || 1;
        var ratio = devicePixelRatio / backingStoreRatio;
        console.log(devicePixelRatio);
        // upscale the canvas if the two ratios don't match
        if (devicePixelRatio !== backingStoreRatio) {
            var oldWidth = canvas.width;
            var oldHeight = canvas.height;
            canvas.width = oldWidth * ratio;
            canvas.height = oldHeight * ratio;
            canvas.style.width = oldWidth + 'px';
            canvas.style.height = oldHeight + 'px';
            // now scale the context to counter
            // the fact that we've manually scaled
            // our canvas element
            ctx.scale(ratio, ratio);
        }
    };
    CandleChart.prototype.onMouseMove = function (evt) {
        if (!this.chart_canvas)
            return;
        var rect = this.chart_canvas.getBoundingClientRect();
        var x = evt.clientX;
        var y = evt.clientY;
        var scaleX = this.chart_canvas.width / rect.width; // relationship bitmap vs. element for X
        var scaleY = this.chart_canvas.height / rect.height; // relationship bitmap vs. element for Y
        this.MOUSE_X = (x - rect.left) * scaleX;
        this.MOUSE_Y = (y - rect.top) * scaleY;
    };
    CandleChart.prototype.getChartRange = function (candles, start_index) {
        var gmax = Number.MIN_VALUE;
        var gmin = Number.MAX_VALUE;
        for (var i = start_index; i < candles.length; ++i) {
            var lmax = Math.max(candles[i].candle.o, candles[i].candle.h, candles[i].candle.l, candles[i].candle.c);
            var lmin = Math.min(candles[i].candle.o, candles[i].candle.h, candles[i].candle.l, candles[i].candle.c);
            if (lmax > gmax)
                gmax = lmax;
            if (lmin < gmin)
                gmin = lmin;
        }
        var r = { max: gmax, min: gmin };
        return r;
    };
    CandleChart.prototype.getPriceWidth = function (ctx) {
        return ctx.measureText(' 0000.0000').width;
    };
    CandleChart.prototype.onMouseWheelEvent = function (evt) {
        console.log(evt.deltaY);
        if (evt.deltaY > 0)
            this.CANDLE_WIDTH += 1;
        if (evt.deltaY < 0)
            this.CANDLE_WIDTH -= 1;
    };
    CandleChart.prototype.drawFull = function (chart) {
        if (!this.chart_canvas) {
            console.error('unable to get canvas element');
            return;
        }
        var ctx = this.chart_canvas.getContext('2d');
        if (!ctx) {
            console.error('contex == undefined');
            return;
        }
        ctx.canvas.width = window.innerWidth;
        ctx.canvas.height = window.innerHeight;
        ctx.font = "normal 1.4em Monospace";
        var drawing_width = this.chart_canvas.width;
        var drawing_height = this.chart_canvas.height - this.PADDING_BOT;
        ctx.strokeStyle = 'white';
        ctx.textBaseline = 'middle';
        ctx.fillStyle = '#131722';
        ctx.fillRect(0, 0, this.chart_canvas.width, this.chart_canvas.height);
        ctx.fillStyle = 'white';
        var candles = chart.chart;
        var num_displayable_candles = (this.chart_canvas.width - this.getPriceWidth(ctx)) / (this.CANDLE_WIDTH + this.CANDLE_SPACING);
        num_displayable_candles -= 1;
        var start_index;
        if (num_displayable_candles < candles.length)
            start_index = Math.floor(candles.length - num_displayable_candles);
        else
            start_index = 0;
        var priceRange = this.getChartRange(candles, start_index);
        ctx.moveTo(drawing_width - this.getPriceWidth(ctx), 0);
        ctx.lineTo(drawing_width - this.getPriceWidth(ctx), this.chart_canvas.height);
        ctx.stroke();
        var pixel_to_price = new LinearEquation(this.PADDING_TOP, priceRange.max, drawing_height - this.PADDING_BOT, priceRange.min);
        var price_to_pixel = new LinearEquation(priceRange.min, drawing_height, priceRange.max, this.PADDING_TOP);
        // DRAW THE PRICE TICKS
        var inc = (priceRange.max - priceRange.min) / this.NUM_TICKS;
        for (var i = priceRange.min; i <= priceRange.max; i += inc) {
            ctx.fillText("-" + (i / 10000).toFixed(4), drawing_width - this.getPriceWidth(ctx), price_to_pixel.eval(i));
            ctx.strokeStyle = 'gray';
            ctx.setLineDash([2, 5]);
            ctx.beginPath();
            ctx.moveTo(0, price_to_pixel.eval(i));
            ctx.lineTo(drawing_width - this.getPriceWidth(ctx), price_to_pixel.eval(i));
            ctx.stroke();
            ctx.setLineDash([]);
        }
        // DRAW THE CANDLES
        var last_color = '';
        for (var i = start_index; i < candles.length; ++i) {
            var width_offset = ((i - start_index) * (this.CANDLE_WIDTH + this.CANDLE_SPACING));
            ctx.fillStyle = 'blue';
            ctx.beginPath();
            ctx.moveTo(width_offset + this.CANDLE_WIDTH / 2.0, price_to_pixel.eval(candles[i].candle.h));
            ctx.lineTo(width_offset + this.CANDLE_WIDTH / 2.0, price_to_pixel.eval(candles[i].candle.l));
            ctx.stroke();
            if (candles[i].candle.o > candles[i].candle.c) {
                ctx.fillStyle = '#EF5350';
                last_color = ctx.fillStyle;
                ctx.fillRect(width_offset, price_to_pixel.eval(candles[i].candle.c), this.CANDLE_WIDTH, price_to_pixel.eval(candles[i].candle.o) -
                    price_to_pixel.eval(candles[i].candle.c));
                ctx.fillStyle = 'black';
            }
            else if (candles[i].candle.o < candles[i].candle.c) {
                ctx.fillStyle = '#26A69A';
                last_color = ctx.fillStyle;
                ctx.fillRect(width_offset, price_to_pixel.eval(candles[i].candle.o), this.CANDLE_WIDTH, price_to_pixel.eval(candles[i].candle.c) -
                    price_to_pixel.eval(candles[i].candle.o));
                ctx.fillStyle = 'black';
            }
            else {
                ctx.fillStyle = 'white';
                ctx.beginPath();
                ctx.moveTo(width_offset, price_to_pixel.eval(candles[i].candle.o));
                ctx.lineTo(width_offset + this.CANDLE_WIDTH, price_to_pixel.eval(candles[i].candle.c));
                ctx.stroke();
            }
        }
        ctx.fillStyle = last_color;
        ctx.fillRect(drawing_width - this.getPriceWidth(ctx), price_to_pixel.eval(candles[candles.length - 1].candle.c) - (20.0 / 2.0), this.getPriceWidth(ctx), 20);
        ctx.fillStyle = 'white';
        ctx.fillText("-" + ((candles[candles.length - 1].candle.c) / 10000.0).toFixed(4), drawing_width - this.getPriceWidth(ctx), price_to_pixel.eval(candles[candles.length - 1].candle.c));
        // draw the mouse cross
        ctx.fillStyle = 'white';
        ctx.fillRect(drawing_width - this.getPriceWidth(ctx), this.MOUSE_Y - (20.0 / 2.0), this.getPriceWidth(ctx), 20);
        ctx.fillStyle = 'black';
        ctx.fillText("-" + ((pixel_to_price.eval(this.MOUSE_Y)) / 10000).toFixed(4), drawing_width - this.getPriceWidth(ctx), this.MOUSE_Y);
        ctx.strokeStyle = 'white';
        ctx.setLineDash([5, 7]);
        ctx.beginPath();
        ctx.moveTo(0, this.MOUSE_Y);
        ctx.lineTo(drawing_width - this.getPriceWidth(ctx), this.MOUSE_Y);
        ctx.moveTo(this.MOUSE_X, 0);
        ctx.lineTo(this.MOUSE_X, drawing_height);
        ctx.stroke();
        ctx.setLineDash([]);
        this.CANDLE_UPDATES_SENT += 1;
        // sync chart every 100 updates
        if (this.CANDLE_UPDATES_SENT % 100 == 0) {
            this.CANDLE_UPDATES_SENT = 0;
            this.conn.send('init|' + this.symbol);
        }
        else {
            this.conn.send('latest|' + this.symbol);
        }
    };
    return CandleChart;
}());
/// <reference path="chart/Chart.ts" />
window.onload = function () {
    var a = new CandleChart("AAPL");
};
/// <reference path="ILinearEquation.ts" />
var LinearEquation = /** @class */ (function () {
    function LinearEquation(x1, y1, x2, y2) {
        this.slope = (y2 - y1) / (x2 - x1);
        this.inter = y1 - (this.slope * x1);
        // y = (m*x) + b
        // y - (mx) = b
    }
    LinearEquation.prototype.eval = function (z) {
        return (this.slope * z) + this.inter;
    };
    return LinearEquation;
}());
