//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowVariableBase.h"

/**
 * Material designed to calculate fluid-phase porepressure and saturation
 * for the single-phase situation, assuming a Gaussian capillary suction
 * function and assuming the independent variable is log(mass density) and
 * assuming the fluid has a constant bulk modulus
 */
class PorousFlow1PhaseMD_Gaussian : public PorousFlowVariableBase
{
public:
  static InputParameters validParams();

  PorousFlow1PhaseMD_Gaussian(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Gaussian parameter: saturation = exp(-(al*p)^2)
  const Real _al;

  /// _al2 = al*al
  const Real _al2;

  /// Fluid density = _dens0*exp(P/_bulk)
  const Real _logdens0;

  /// Fluid density = _dens0*exp(P/_bulk)
  const Real _bulk;

  /// 1/_bulk/_al
  const Real _recip_bulk;

  /// (1/_bulk)^2
  const Real _recip_bulk2;

  /// Nodal or quadpoint value of mass-density of the fluid phase
  const VariableValue & _md_var;

  /// Gradient(_mass-density at quadpoints)
  const VariableGradient & _gradmd_qp_var;

  /// Moose variable number of the mass-density
  const unsigned int _md_varnum;

  /// PorousFlow variable number of the mass-density
  const unsigned int _pvar;

  virtual void buildPS();
};
