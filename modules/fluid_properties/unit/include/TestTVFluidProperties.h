//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SinglePhaseFluidProperties.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Analytic fluid defined in terms of (T, v) whose enthalpy depends on the specific volume,
 * used to test mixture models that combine component (T, v) derivatives:
 *   p = a T / v + b / v^2,  h = c T + d / v
 */
class TestTVFluidProperties : public SinglePhaseFluidProperties
{
public:
  static InputParameters validParams();

  TestTVFluidProperties(const InputParameters & parameters);

  virtual Real p_from_T_v(Real T, Real v) const override;
  virtual void p_from_T_v(Real T, Real v, Real & p, Real & dp_dT, Real & dp_dv) const override;

  virtual Real h_from_T_v(Real T, Real v) const override;
  virtual void h_from_T_v(Real T, Real v, Real & h, Real & dh_dT, Real & dh_dv) const override;

protected:
  const Real _a;
  const Real _b;
  const Real _c;
  const Real _d;
};

#pragma GCC diagnostic pop
