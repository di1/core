/// <reference path="ILinearEquation.ts" />

class LinearEquation {

  slope: number;
  inter: number;
  constructor(x1: number, y1: number, x2: number, y2: number) {
    this.slope = (y2-y1)/(x2-x1);
    this.inter = y1 - (this.slope*x1);


    // y = (m*x) + b
    // y - (mx) = b

  }

  public eval(z: number): number {
    return (this.slope*z) + this.inter;
  }

}
