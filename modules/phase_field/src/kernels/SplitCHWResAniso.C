//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplitCHWResAniso.h"

registerMooseObject("PhaseFieldApp", SplitCHWResAniso);

InputParameters
SplitCHWResAniso::validParams()
{
  InputParameters params = SplitCHWResBase<RealTensorValue>::validParams();
  params.addClassDescription("Split formulation Cahn-Hilliard Kernel for the chemical potential "
                             "variable with a tensor (anisotropic) mobility");
  return params;
}

SplitCHWResAniso::SplitCHWResAniso(const InputParameters & parameters)
  : SplitCHWResBase<RealTensorValue>(parameters)
{
}
