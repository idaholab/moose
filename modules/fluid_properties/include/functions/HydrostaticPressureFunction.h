//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "GravityVectorInterface.h"

/**
 * Computes hydrostatic pressure from a reference point.
 */
class HydrostaticPressureFunction : public Function, public GravityVectorInterface
{
public:
  static InputParameters validParams();

  HydrostaticPressureFunction(const InputParameters & parameters);

  virtual void initialSetup() override;

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

protected:
  /// Reference pressure
  const Real _p_ref;
  /// Reference temperature
  const Real _T_ref;
  /// Reference point
  const Point & _r_ref;

  /// Density at reference pressure and temperature
  Real _rho;
};
