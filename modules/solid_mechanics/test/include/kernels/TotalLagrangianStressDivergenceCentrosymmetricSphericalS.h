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
TotalLagrangianStressDivergenceBaseS<GradientOperatorCentrosymmetricSpherical>::validParams()
{
  InputParameters params = TotalLagrangianStressDivergenceBaseS::baseParams();
  params.addClassDescription("Enforce equilibrium with a total Lagrangian formulation in "
                             "centrosymmetric spherical coordinates.");
  return params;
}

template <>
void
TotalLagrangianStressDivergenceBaseS<GradientOperatorCentrosymmetricSpherical>::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RSPHERICAL)
    mooseError("This kernel should only act in centrosymmetric spherical coordinates.");
}

typedef TotalLagrangianStressDivergenceBaseS<GradientOperatorCentrosymmetricSpherical>
    TotalLagrangianStressDivergenceCentrosymmetricSphericalS;
