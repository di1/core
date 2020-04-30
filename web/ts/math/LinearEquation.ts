/**
  Represents a linear equation
 */
class LinearEquation { // eslint-disable-line no-unused-vars
  slope: number;
  inter: number;

  /**
    Constructs a linear equation given two points

    @param {number} x1 Point 1 X coordiant
    @param {number} y1 Point 1 Y coordiant
    @param {number} x2 Point 2 X coordiant
    @param {number} y2 Point 2 Y coordiant
   */
  constructor(x1: number, y1: number, x2: number, y2: number) {
    this.slope = (y2-y1)/(x2-x1);
    this.inter = y1 - (this.slope*x1);
  }

  /**
    Evaluates the linear equation given an x coordinant

    @param {number} z The x coordiant to evaluate the line
    @return {number} Returns the value of m*x + b
   */
  public eval(z: number): number {
    return (this.slope*z) + this.inter;
  }
}
