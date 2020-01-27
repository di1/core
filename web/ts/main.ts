window.onload = () => {
  let greeter: HTMLElement | null = document.getElementById("greeter");

  if (greeter) {
    greeter.innerText = "Hello World!";
  }
}
