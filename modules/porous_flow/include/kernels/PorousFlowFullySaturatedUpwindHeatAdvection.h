//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowDarcyBase.h"

/**
 * Advection of heat via flux of a single-phase fluid.
 * A fully-updwinded version is implemented, where the mobility
 * of the upstream nodes is used.
 */
class PorousFlowFullySaturatedUpwindHeatAdvection : public PorousFlowDarcyBase
{
public:
  static InputParameters validParams();

  PorousFlowFullySaturatedUpwindHeatAdvection(const InputParameters & parameters);

protected:
  virtual Real mobility(unsigned nodenum, unsigned phase) const override;
  virtual Real dmobility(unsigned nodenum, unsigned phase, unsigned pvar) const override;

  /// Enthalpy of each phase
  const MaterialProperty<std::vector<Real>> & _enthalpy;

  /// Derivative of the enthalpy wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _denthalpy_dvar;
};
