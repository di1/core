/// <reference path="chart/Chart.ts" />

let large_display_chart: CandleChart | null = null;

function search_input_key_press(evt: KeyboardEvent) {
  if (evt.keyCode != 13) {
    return;
  }

  let search_input: EventTarget | null = evt.srcElement;
  let wanted_stock = (<HTMLInputElement> search_input).value.toUpperCase();
  console.log(wanted_stock);

  if (!large_display_chart) {
    console.error('display chart is undefined');
    return;
  }
  large_display_chart.setSymbol(wanted_stock);
}

window.onload = () => {
  large_display_chart = new CandleChart("AAPL");

  let search_input: HTMLInputElement | null =
  <HTMLInputElement> document.getElementById('stock-search-input');

  if (!search_input) {
    console.error("can't find search input");
    return;
  }
  search_input.onkeypress = search_input_key_press;
}

