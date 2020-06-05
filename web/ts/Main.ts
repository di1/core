/**
  Start the loop
 */
window.onload = () => {
  // set the style
  document.body.style.backgroundColor = DarkTheme.bg;
  document.body.style.color = DarkTheme.fg;
  document.body.style.margin = '0';
  document.body.style.padding = '0';
  document.body.style.width = '100%';
  document.body.style.height = '100%';
  document.body.style.position = 'absolute';
  document.body.style.top = '0';
  document.body.style.left = '0';
  document.body.style.fontFamily = '\'Inconsolata\', monospace';

  new Chart('OANDA', 'USD_JPY', DarkTheme);
};
