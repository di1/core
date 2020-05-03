/**
  Represents a linear equation
 */
class LinearEquation { // eslint-disable-line no-unused-vars
  private slope: number;
  private inter: number;

  /**
    The first x coordiant used to build the line
   */
  public x1: number;

  /**
    The first y coordiant used to build the line
   */
  public y1: number;

  /**
    The second x coordiant used to build the line
   */
  public x2: number;

  /**
    The second y coordiant used to build the line
   */
  public y2: number;

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
    this.x1 = x1;
    this.y1 = y1;
    this.x2 = x2;
    this.y2 = y2;
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
