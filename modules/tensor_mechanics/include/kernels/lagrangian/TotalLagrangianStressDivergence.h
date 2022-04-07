//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TotalLagrangianStressDivergenceBase.h"

template <>
inline InputParameters
TotalLagrangianStressDivergenceBase<GradientOperatorCartesian>::validParams()
{
  InputParameters params = TotalLagrangianStressDivergenceBase::baseParams();
  params.addClassDescription(
      "Enforce equilibrium with a total Lagrangian formulation in Cartesian coordinates.");
  return params;
}

template <>
inline void
TotalLagrangianStressDivergenceBase<GradientOperatorCartesian>::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_XYZ)
    mooseError("This kernel should only act in Cartesian coordinates.");
}

typedef TotalLagrangianStressDivergenceBase<GradientOperatorCartesian>
    TotalLagrangianStressDivergence;
