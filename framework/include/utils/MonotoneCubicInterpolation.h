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

#ifndef MONOTONECUBICINTERPOLATION_H
#define MONOTONECUBICINTERPOLATION_H

#include "MooseEnum.h"

#include <vector>

class MonotoneCubicInterpolation
{
public:
  MonotoneCubicInterpolation();
  MonotoneCubicInterpolation(const std::vector<double> & x, const std::vector<double> & y);

  virtual ~MonotoneCubicInterpolation() = default;

  virtual void setData(const std::vector<double> & x, const std::vector<double> & y);
  virtual double sample(const double & x) const;
  virtual double sampleDerivative(const double & x) const;
  virtual double sample2ndDerivative(const double & x) const;
  virtual void dumpCSV(std::string filename, const std::vector<double> & xnew);

protected:

  virtual void errorCheck();
  double sign(const double & x) const;
  enum MonotonicStatus {monotonic_increase, monotonic_decrease, monotonic_constant, monotonic_not};
  MonotonicStatus _monotonic_status;
  void checkMonotone();

  double phi(const double & t) const;
  double psi(const double & t) const;
  double phiPrime(const double & t) const;
  double psiPrime(const double & t) const;
  double phiDoublePrime(const double & t) const;
  double psiDoublePrime(const double & t) const;

  /// Cubic hermite polynomials
  double h1(const double & xhi, const double & xlo, const double & x) const;
  double h2(const double & xhi, const double & xlo, const double & x) const;
  double h3(const double & xhi, const double & xlo, const double & x) const;
  double h4(const double & xhi, const double & xlo, const double & x) const;
  double h1Prime(const double & xhi, const double & xlo, const double & x) const;
  double h2Prime(const double & xhi, const double & xlo, const double & x) const;
  double h3Prime(const double & xhi, const double & xlo, const double & x) const;
  double h4Prime(const double & xhi, const double & xlo, const double & x) const;
  double h1DoublePrime(const double & xhi, const double & xlo, const double & x) const;
  double h2DoublePrime(const double & xhi, const double & xlo, const double & x) const;
  double h3DoublePrime(const double & xhi, const double & xlo, const double & x) const;
  double h4DoublePrime(const double & xhi, const double & xlo, const double & x) const;

  /// Interpolating cubic polynomial and derivatives
  virtual double p(const double & xhi, const double & xlo, const double & fhi, const double & flo,
                 const double & dhi, const double & dlo, const double & x) const;
  virtual double pPrime(const double & xhi, const double & xlo, const double & fhi, const double & flo,
                      const double & dhi, const double & dlo, const double & x) const;
  virtual double pDoublePrime(const double & xhi, const double & xlo, const double & fhi, const double & flo,
                            const double & dhi, const double & dlo, const double & x) const;

  virtual void initialize_derivs();
  virtual void modify_derivs(const double & alpha, const double & beta, const double & delta, double & yp_lo, double & yp_hi);
  virtual void solve();
  virtual void findInterval(const double & x, unsigned int & klo, unsigned int & khi) const;

  std::vector<double> _x;
  std::vector<double> _y;
  std::vector<double> _h;
  std::vector<double> _yp;
  std::vector<double> _delta;
  std::vector<double> _alpha;
  std::vector<double> _beta;

  unsigned int _n_knots;
  unsigned int _n_intervals;
};

#endif
