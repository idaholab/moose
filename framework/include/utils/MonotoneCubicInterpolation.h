//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>

#include "libmesh/libmesh_common.h"

using libMesh::Real;

/**
 * This class interpolates values given a set of data pairs and an abscissa.
 * The interpolation is cubic with at least C1 continuity; C2 continuity can
 * be violated in favor of ensuring the interpolation is monotonic.
 * The algorithm used is laid out in Fritsch and Carlson, SIAM J. Numer. Anal.
 * Vol. 17(2) April 1980
 */
class MonotoneCubicInterpolation
{
public:
  /**
   * Empty constructor
   */
  MonotoneCubicInterpolation();

  /**
   * Constructor, Takes two vectors of points for which to apply the fit.  One
   * should be of the independent variable while the other should be of the
   * dependent variable. These values should have a one-to-one correspondence,
   * e.g. the vectors must be of the same size.
   */
  MonotoneCubicInterpolation(const std::vector<Real> & x, const std::vector<Real> & y);

  virtual ~MonotoneCubicInterpolation() = default;

  /**
   * Method generally used when MonotoneCubicInterpolation object was created
   * using the empty constructor. Takes two vectors of points for which to apply
   * the fit.  One should be of the independent variable while the other should
   * be of the dependent variable. These values should have a one-to-one
   * correspondence, e.g. the vectors must be of the same size.
   */
  virtual void setData(const std::vector<Real> & x, const std::vector<Real> & y);

  /**
   * This function will take an independent variable input and will return the dependent
   * variable based on the generated fit.
   */
  virtual Real sample(const Real & x) const;

  /**
   * This function will take an independent variable input and will return the derivative
   * of the dependent variable with respect to the independent variable based on the
   * generated fit.
   */
  virtual Real sampleDerivative(const Real & x) const;

  /**
   * This function will take an independent variable input and will return the second
   * derivative of the dependent variable with respect to the independent variable
   * based on the generated fit. Note that this can be discontinous at the knots.
   */
  virtual Real sample2ndDerivative(const Real & x) const;

  /**
   * This function takes an array of independent variable values and writes a CSV file
   * with values corresponding to y, y', and y''. This can be used for sanity checks
   * of the interpolation curve.
   */
  virtual void dumpCSV(std::string filename, const std::vector<Real> & xnew);

  /**
   * This method returns the length of the independent variable vector
   */
  virtual unsigned int getSampleSize();

protected:
  // Error check routines run during initialization
  virtual void errorCheck();
  Real sign(const Real & x) const;

  // Building blocks of Hermite polynomials
  Real phi(const Real & t) const;
  Real psi(const Real & t) const;
  Real phiPrime(const Real & t) const;
  Real psiPrime(const Real & t) const;
  Real phiDoublePrime(const Real & t) const;
  Real psiDoublePrime(const Real & t) const;

  // Cubic Hermite polynomials
  Real h1(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h2(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h3(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h4(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h1Prime(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h2Prime(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h3Prime(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h4Prime(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h1DoublePrime(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h2DoublePrime(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h3DoublePrime(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h4DoublePrime(const Real & xhi, const Real & xlo, const Real & x) const;

  // Interpolating cubic polynomial and derivatives
  virtual Real p(const Real & xhi,
                 const Real & xlo,
                 const Real & fhi,
                 const Real & flo,
                 const Real & dhi,
                 const Real & dlo,
                 const Real & x) const;
  virtual Real pPrime(const Real & xhi,
                      const Real & xlo,
                      const Real & fhi,
                      const Real & flo,
                      const Real & dhi,
                      const Real & dlo,
                      const Real & x) const;
  virtual Real pDoublePrime(const Real & xhi,
                            const Real & xlo,
                            const Real & fhi,
                            const Real & flo,
                            const Real & dhi,
                            const Real & dlo,
                            const Real & x) const;

  // Algorithm routines
  virtual void initialize_derivs();
  virtual void modify_derivs(
      const Real & alpha, const Real & beta, const Real & delta, Real & yp_lo, Real & yp_hi);
  virtual void solve();
  virtual void findInterval(const Real & x, unsigned int & klo, unsigned int & khi) const;

  std::vector<Real> _x;
  std::vector<Real> _y;
  std::vector<Real> _h;
  std::vector<Real> _yp;
  std::vector<Real> _delta;
  std::vector<Real> _alpha;
  std::vector<Real> _beta;

  unsigned int _n_knots;
  unsigned int _n_intervals;
  unsigned int _internal_knots;
};
