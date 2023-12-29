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
ComputeLagrangianStrainBase<GradientOperatorCartesian>::validParams()
{
  InputParameters params = ComputeLagrangianStrainBase::baseParams();
  params.addClassDescription("Compute strain in Cartesian coordinates.");
  return params;
}

template <>
inline void
ComputeLagrangianStrainBase<GradientOperatorCartesian>::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_XYZ)
    mooseError("This kernel should only act in Cartesian coordinates.");
}

typedef ComputeLagrangianStrainBase<GradientOperatorCartesian> ComputeLagrangianStrain;
