//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialVectorBase.h"

/**
 * Base class Material designed to provide the density of the porous medium
 */
class PorousFlowTotalGravitationalDensityBase : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowTotalGravitationalDensityBase(const InputParameters & parameters);

protected:
  /// Computed density at quadpoints
  MaterialProperty<Real> & _gravdensity;

  /// d(density)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real>> & _dgravdensity_dvar;
};
