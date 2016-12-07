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
#include "libmesh/libmesh_common.h"
using libMesh::Real;

MooseEnum monotonic_e ("monotonic_increase montonic_decrease monontonic_constant monotonic_not");

class MonotoneCubicInterpolation
{
public:
  MonotoneCubicInterpolation();
  MonotoneCubicInterpolation(const std::vector<Real> x, const std::vector<Real> y);
  
  virtual ~MonotoneCubicInterpolation() = default;

  virtual Real sample(Real x) const;
  virtual Real sampleDerivative(Real x) const;
  // Real sampleDerivative(const std::vector<Real> & x, const std::vector<Real> & y, const std::vector<Real> & y2, Real x_int) const;
  // Real sample2ndDerivative(const std::vector<Real> & x, const std::vector<Real> & y, const std::vector<Real> & y2, Real x_int) const;

protected:

  virtual void errorCheck();
  Real sign(Real x) const;
  MooseEnum _monotonic_e = MooseEnum ("monotonic_increase montonic_decrease monontonic_constant monotonic_not");
  void checkMonotone();
  
  Real phi(const Real & t) const;
  Real psi(const Real & t) const;
  Real phiPrime(const Real & t) const;
  Real psiPrime(const Real & t) const;
  
  /// Cubic hermite polynomials
  Real h1(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h2(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h3(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h4(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h1Prime(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h2Prime(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h3Prime(const Real & xhi, const Real & xlo, const Real & x) const;
  Real h4Prime(const Real & xhi, const Real & xlo, const Real & x) const;

  /// Interpolating cubic polynomial
  virtual Real p(const Real & xhi, const Real & xlo, const Real & fhi, const Real & flo,
                 const Real & dhi, const Real & dlo, const Real & x) const;
  virtual Real pPrime(const Real & xhi, const Real & xlo, const Real & fhi, const Real & flo,
                 const Real & dhi, const Real & dlo, const Real & x) const;

  virtual void initialize_derivs();
  virtual void modify_derivs(const Real & alpha, const Real & beta, const Real & delta, Real & yp_lo, Real & yp_hi);
  virtual void solve();
  
  const std::vector<Real> _x;
  const std::vector<Real> _y;
  std::vector<Real> _h;
  std::vector<Real> _yp;
  std::vector<Real> _delta;
  std::vector<Real> _alpha;
  std::vector<Real> _beta;
  
  unsigned int _n_knots;
  unsigned int _n_intervals;
};

#endif
