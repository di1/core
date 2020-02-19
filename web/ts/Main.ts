let largeDisplayChart: CandleChart | null = null;

/**
  Callback for key press on stock search input.
  On enter will switch the largeDisplayChart with
  the stock symbol entered.

  @param {KeyboardEvent} evt The keyboard event
 */
function searchInputKeyPress(evt: KeyboardEvent) {
  if (evt.keyCode != 13) {
    return;
  }

  const searchInput: EventTarget | null = evt.srcElement;
  const wantedStock = (<HTMLInputElement> searchInput).value.toUpperCase();
  console.log(wantedStock);

  if (!largeDisplayChart) {
    console.error('display chart is undefined');
    return;
  }
  largeDisplayChart.setSymbol(wantedStock);
}

window.onload = () => {
  largeDisplayChart = new CandleChart('AAPL');

  const searchInput: HTMLInputElement | null =
  <HTMLInputElement> document.getElementById('stock-search-input');

  if (!searchInput) {
    console.error('can\'t find search input');
    return;
  }
  searchInput.onkeypress = searchInputKeyPress;
};

