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

#ifndef POLYNOMIALFIT_H
#define POLYNOMIALFIT_H

#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include "Moose.h"

/**
 * This class applies the Least Squares algorithm to a set of points to provide a smooth curve for
 * sampling values.
 *
 * Requires: LAPACK
 */
class PolynomialFit
{
public:

  /* Constructor, Takes two vectors of points for which to apply the fit.  One should be of the
   * independent variable while the other should be of the dependent variable.  These values should
   * correspond to one and other in the same position.  The third parameter is the requested polynomial
   * order and the forth parameter tells the class whether or not it should truncate the order if there
   * are not enough points for which to apply the polynomial fit.
   */
  PolynomialFit(std::vector<Real> X, std::vector<Real> Y, unsigned int order, bool truncate_order = false);

  virtual ~PolynomialFit()
    {}

  /**
   * This function generates the polynomial fit.  This function must be called prior to using
   * sample or dumpSample File. Note:  If you pass a vectors that
   * contain duplicate independent measures the call to LAPACK will fail
   */
  void generate();

  /**
   * This function will take an independent variable input and will return the dependent variable
   * based on the generated fit
   */
  Real sample(Real x);

  /**
   * This function will dump GNUPLOT input files that can be run to show the data points and
   * function fits
   */
  void dumpSampleFile(std::string base_name, std::string x_label="X", std::string y_label="Y", float xmin=0, float xmax=0, float ymin=0, float ymax=0);

  /**
   * This function returns the size of the array holding the points, i.e. the number of sample points
   */
  unsigned int getSampleSize();
  
private:

  /**
   * This is a helper function that creates the matrix necessary for the Least Squares algorithm.
   */
  void fillMatrix();

  /**
   * This function is the wrapper for the LAPACK dgels function and is called by generate.
   */
  void doLeastSquares();

  std::vector<Real> _x;
  std::vector<Real> _y;
  std::vector<Real> _matrix;
  std::vector<Real> _coeffs;
  unsigned int _order;
  bool _truncate_order;

  static int _file_number;
};

#endif //POLYNOMIALFIT_H
