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
TotalLagrangianStressDivergenceBase<GradientOperatorAxisymmetricCylindrical>::validParams()
{
  InputParameters params = TotalLagrangianStressDivergenceBase::baseParams();
  params.addClassDescription("Enforce equilibrium with a total Lagrangian formulation in "
                             "axisymmetric cylindrical coordinates.");
  return params;
}

template <>
void
TotalLagrangianStressDivergenceBase<GradientOperatorAxisymmetricCylindrical>::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("This kernel should only act in axisymmetric cylindrical coordinates.");
}

typedef TotalLagrangianStressDivergenceBase<GradientOperatorAxisymmetricCylindrical>
    TotalLagrangianStressDivergenceAxisymmetricCylindrical;
