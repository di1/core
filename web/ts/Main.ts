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
  if (evt.keyCode == 13) {
    console.log(wantedStock);

    const searchResultDiv: HTMLDivElement | null =
      <HTMLDivElement> document.getElementById('search-results');
    searchResultDiv.style.display = 'none';

    if (!largeDisplayChart) {
      console.error('display chart is undefined');
      return;
    }
    largeDisplayChart.setSymbol(wantedStock);
  } else {
    if (searchSocket && wantedStock !== '') {
      searchSocket.send('search|' + wantedStock);
    }
  }
}

/**
  Callback for search results
  @param {MessageEvent} evt The message event
 */
function onSearchReceived(evt: MessageEvent) {
  console.log(evt.data);
  try {
    const searchResult = JSON.parse(evt.data);
    console.log(searchResult);
    const searchResultDiv: HTMLDivElement | null =
      <HTMLDivElement> document.getElementById('search-results');

    const searchInput: HTMLInputElement | null =
      <HTMLInputElement> document.getElementById('stock-search-input');

    const inputRect: DOMRect = searchInput.getBoundingClientRect();

    searchResultDiv.style.top = (inputRect.bottom).toString() + 'px';
    searchResultDiv.style.left = (inputRect.left).toString() + 'px';
    searchResultDiv.style.width = (inputRect.width).toString() + 'px';
    searchResultDiv.style.height = '200px';
    searchResultDiv.style.display = 'block';

    searchResultDiv.innerHTML = '';

    let inter: number = <number> searchResult.length;
    if (inter >= 10) {
      inter = 10;
    }
    for (let i = 0; i < inter; ++i) {
      searchResultDiv.innerHTML +=
        '<span class="search-row"><span>' + searchResult[i]['symbol'] +
        '</span><span>' + searchResult[i]['fullName'];
    }

    console.log(searchResultDiv);
  } catch (err) {
    console.log('whoot');
  }
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

