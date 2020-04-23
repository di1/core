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
  const wantedStock = (<HTMLInputElement> searchInput).value.toUpperCase();

  if (evt.keyCode == 13) {
    const searchResultDiv: HTMLDivElement | null =
      <HTMLDivElement> document.getElementById('search-results');
    searchResultDiv.style.display = 'none';

    if (!largeDisplayChart) {
      console.error('display chart is undefined');
      return;
    }

    largeDisplayChart.setSymbol(wantedStock);
    (<HTMLInputElement> searchInput).value = '';

    const tabElements: HTMLCollectionOf<HTMLSpanElement> | null =
    <HTMLCollectionOf<HTMLSpanElement>>
    document.getElementsByClassName('chart-tab');

    for (let i: number = 0; i < tabElements.length; ++i) {
      tabElements[i].style.backgroundColor = '#cc858563';
      tabElements[i].style.color = 'black';
    }

    const tabBar: HTMLDivElement | null =
      <HTMLDivElement> document.getElementById('chart-tabs');

    tabBar.insertAdjacentHTML('beforeend',
        '<span class="chart-tab"><span>' +
            wantedStock + '</span><span>&times;</span>',
    );
    if (tabBar.lastChild) {
      (<HTMLSpanElement> tabBar.lastChild).style.backgroundColor = '#212733';
      (<HTMLSpanElement> tabBar.lastChild).style.color = '#f0f0f0';
    }

    const tabElementsParents: NodeListOf<HTMLSpanElement> | null =
    <NodeListOf<HTMLSpanElement>>
    document.querySelectorAll('.chart-tab > span:first-child');

    for (let i: number = 0; i < tabElementsParents.length; ++i) {
      tabElementsParents[i].onclick = tabOnClick;

      (<HTMLSpanElement>(<HTMLSpanElement> tabElementsParents[i].parentNode).
          lastElementChild).onclick = deleteTabOnClick;
    }
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
  try {
    const searchResult = JSON.parse(evt.data);
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
        '<span class="search-row">' +
        ' <span>' + searchResult[i]['symbol'] + '</span>'+
        ' <span>' + searchResult[i]['fullName'] + '</span>'+
        '</span>';
    }
  } catch (err) {
  }
}

/**
 * Changes the full screen window the the symbol in the tab
 * @param {MouseEvent} evt The mouse event
 */
function tabOnClick(evt: MouseEvent) {
  const toChangeSpanChild = <HTMLSpanElement> (evt.srcElement);
  const toChangeSpan = <HTMLSpanElement> toChangeSpanChild.parentElement;

  const tabElements: HTMLCollectionOf<HTMLSpanElement> | null =
    <HTMLCollectionOf<HTMLSpanElement>>
    document.getElementsByClassName('chart-tab');

  for (let i: number = 0; i < tabElements.length; ++i) {
    tabElements[i].style.backgroundColor = '#cc858563';
    tabElements[i].style.color = 'black';
  }

  toChangeSpan.style.backgroundColor = '#212733';
  toChangeSpan.style.color = '#f0f0f0';

  if (largeDisplayChart) {
    largeDisplayChart.setSymbol(toChangeSpanChild.innerText);
  }
}

/**
 * Deletes a tab on click of the X
 * @param {MouseEvent} evt The mouse event
 */
function deleteTabOnClick(evt: MouseEvent) {
  const toDeleteSpanChild = <HTMLSpanElement> (evt.srcElement);
  const toDeleteSpan = <HTMLSpanElement> (toDeleteSpanChild.parentElement);

  toDeleteSpan.remove();
}

window.onload = () => {
  // create a new websocket for searching
  searchSocket = new WebSocket('ws://' + window.location.hostname + ':7681', 'lws-minimal');
  searchSocket.onmessage = onSearchReceived;

  largeDisplayChart = new CandleChart('AMD');

  const searchInput: HTMLInputElement | null =
  <HTMLInputElement> document.getElementById('stock-search-input');

  if (!searchInput) {
    console.error('can\'t find search input');
    return;
  }
  searchInput.onkeyup = searchInputKeyPress;

  const tabElements: NodeListOf<HTMLSpanElement> | null =
    <NodeListOf<HTMLSpanElement>>
    document.querySelectorAll('.chart-tab > span:first-child');

  for (let i: number = 0; i < tabElements.length; ++i) {
    if (i == 0) {
      (<HTMLSpanElement> tabElements[i].parentNode).style.backgroundColor =
        '#212733';
      (<HTMLSpanElement> tabElements[i].parentNode).style.color = 'white';
    }
    tabElements[i].onclick = tabOnClick;

    (<HTMLSpanElement>
     (<HTMLSpanElement> tabElements[i].parentNode)
         .lastElementChild).onclick = deleteTabOnClick;
  }
};
