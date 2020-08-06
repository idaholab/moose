//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Calculate the kinetic mineral species kinetic rate according to transient
 * state theory rate law
 */
class KineticDisPreRateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  KineticDisPreRateAux(const InputParameters & parameters);

  virtual ~KineticDisPreRateAux() {}

protected:
  virtual Real computeValue() override;

  /// Equilibrium constant at reference temperature
  const VariableValue & _log_k;
  /// Specific reactive surface area, m^2/L solution
  const Real _r_area;
  /// Reference kinetic rate constant
  const Real _ref_kconst;
  /// Activation energy
  const Real _e_act;
  /// Gas constant, 8.314 J/mol/K
  const Real _gas_const;
  /// Reference temperature
  const Real _ref_temp;
  /// Actual system temperature
  const VariableValue & _sys_temp;
  /// Stoichiometric coefficients for involved primary species
  const std::vector<Real> _sto_v;
  /// Coupled primary species concentrations
  const std::vector<const VariableValue *> _vals;
};
