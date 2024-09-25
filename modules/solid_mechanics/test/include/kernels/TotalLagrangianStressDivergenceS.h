//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TotalLagrangianStressDivergenceBaseS.h"

template <>
inline InputParameters
TotalLagrangianStressDivergenceBaseS<GradientOperatorCartesian>::validParams()
{
  InputParameters params = TotalLagrangianStressDivergenceBaseS::baseParams();
  params.addClassDescription(
      "Enforce equilibrium with a total Lagrangian formulation in Cartesian coordinates.");
  return params;
}

template <>
inline void
TotalLagrangianStressDivergenceBaseS<GradientOperatorCartesian>::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_XYZ)
    mooseError("This kernel should only act in Cartesian coordinates.");
}

typedef TotalLagrangianStressDivergenceBaseS<GradientOperatorCartesian>
    TotalLagrangianStressDivergenceS;

extern template class TotalLagrangianStressDivergenceBaseS<GradientOperatorCartesian>;
