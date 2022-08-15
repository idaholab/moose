//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStrainBase.h"

template <>
inline InputParameters
ComputeLagrangianStrainBase<GradientOperatorCentrosymmetricSpherical>::validParams()
{
  InputParameters params = ComputeLagrangianStrainBase::baseParams();
  params.addClassDescription("Compute strain in centrosymmetric spherical coordinates.");
  return params;
}

template <>
void
ComputeLagrangianStrainBase<GradientOperatorCentrosymmetricSpherical>::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RSPHERICAL)
    mooseError("This kernel should only act in centrosymmetric spherical coordinates.");
}

typedef ComputeLagrangianStrainBase<GradientOperatorCentrosymmetricSpherical>
    ComputeLagrangianStrainCentrosymmetricSpherical;
