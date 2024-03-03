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
ComputeLagrangianStrainBase<GradientOperatorAxisymmetricCylindrical>::validParams()
{
  InputParameters params = ComputeLagrangianStrainBase::baseParams();
  params.addClassDescription("Compute strain in 2D axisymmetric RZ coordinates.");
  return params;
}

template <>
void
ComputeLagrangianStrainBase<GradientOperatorAxisymmetricCylindrical>::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("This kernel should only act in 2D axisymmetric RZ coordinates.");
}

typedef ComputeLagrangianStrainBase<GradientOperatorAxisymmetricCylindrical>
    ComputeLagrangianStrainAxisymmetricCylindrical;
