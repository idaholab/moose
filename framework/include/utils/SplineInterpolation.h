/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef SPLINEINTERPOLATION_H
#define SPLINEINTERPOLATION_H

#include <vector>
#include <fstream>
#include <sstream>
#include <string>


/**
 * This class interpolates tabulated functions with cubic splines
 *
 * Adopted from Numerical Recipes in C (section 3.3).
 */
class SplineInterpolation
{
public:
  SplineInterpolation();
  /**
   * Construct the object
   * @param x Tabulated function (x-positions)
   * @param y Tabulated function (y-positions)
   * @param yp1 First derivative of the interpolating function at point 1
   * @param ypn First derivative of the interpolating function at point n
   *
   * If yp1, ypn are not specified or greater or equal that 1e30, we use natural spline
   */
  SplineInterpolation(const std::vector<double> & x, const std::vector<double> & y, double yp1 = 1e30, double ypn = 1e30);

  virtual ~SplineInterpolation() { }

  /**
   * Set the x-, y- values and first derivatives
   */
  void setData(const std::vector<double> & x, const std::vector<double> & y, double yp1 = 1e30, double ypn = 1e30);

  void errorCheck();

  /**
   * This function will take an independent variable input and will return the dependent variable
   * based on the generated fit
   */
  double sample(double x) const;

  double sampleDerivative(double x) const;

  double sample2ndDerivative(double x) const;

  /**
   * This function will dump GNUPLOT input files that can be run to show the data points and
   * function fits
   */
  void dumpSampleFile(std::string base_name, std::string x_label="X", std::string y_label="Y", float xmin=0, float xmax=0, float ymin=0, float ymax=0);

  /**
   * This function returns the size of the array holding the points, i.e. the number of sample points
   */
  unsigned int getSampleSize();

  double domain(int i) const;
  double range(int i) const;

protected:
  std::vector<double> _x;
  std::vector<double> _y;
  /// boundary conditions
  double _yp1, _ypn;
  /// second derivatives of the interpolating function
  std::vector<double> _y2;

  void solve();

  void findInterval(double x, unsigned int & klo, unsigned int & khi) const;
  void computeCoeffs(unsigned int klo, unsigned int khi, double x, double & h, double & a, double & b) const;

  static int _file_number;
};

#endif //LINEARINTERPOLATION_H




