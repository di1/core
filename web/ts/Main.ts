let largeDisplayChart: CandleChart | null = null;
let searchSocket: WebSocket | null = null;

/**
  Callback for key press on stock search input.
  On enter will switch the largeDisplayChart with
  the stock symbol entered.

  @param {KeyboardEvent} evt The keyboard event
 */
function searchInputKeyPress(evt: KeyboardEvent) {
  const searchInput: EventTarget | null = evt.srcElement;
  console.log(searchInput);
  const wantedStock = (<HTMLInputElement> searchInput).value.toUpperCase();

  console.log(wantedStock);
  if (searchSocket && wantedStock !== '') {
    searchSocket.send('search|' + wantedStock);
  }

  if (evt.keyCode != 13) {
    return;
  }

  console.log(wantedStock);

  if (!largeDisplayChart) {
    console.error('display chart is undefined');
    return;
  }
  largeDisplayChart.setSymbol(wantedStock);
}

/**
  Callback for search results
  @param {MessageEvent} evt The message event
 */
function onSearchReceived(evt: MessageEvent) {
  console.log('search results: ' + evt.data);
}

window.onload = () => {
  // create a new websocket for searching
  searchSocket = new WebSocket('ws://riski.local:7681', 'lws-minimal');
  searchSocket.onmessage = onSearchReceived;

  largeDisplayChart = new CandleChart('AAPL');

  const searchInput: HTMLInputElement | null =
  <HTMLInputElement> document.getElementById('stock-search-input');

  if (!searchInput) {
    console.error('can\'t find search input');
    return;
  }
  searchInput.onkeyup = searchInputKeyPress;
};

