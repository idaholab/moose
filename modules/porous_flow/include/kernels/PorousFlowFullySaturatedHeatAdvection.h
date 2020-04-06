//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowFullySaturatedDarcyBase.h"

/**
 * Advection of heat via flux via Darcy flow of a single phase
 * fully-saturated fluid.  No upwinding is used.
 */
class PorousFlowFullySaturatedHeatAdvection : public PorousFlowFullySaturatedDarcyBase
{
public:
  static InputParameters validParams();

  PorousFlowFullySaturatedHeatAdvection(const InputParameters & parameters);

protected:
  virtual Real mobility() const override;
  virtual Real dmobility(unsigned pvar) const override;

  /// Enthalpy of each phase
  const MaterialProperty<std::vector<Real>> & _enthalpy;

  /// Derivative of the enthalpy wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _denthalpy_dvar;
};
