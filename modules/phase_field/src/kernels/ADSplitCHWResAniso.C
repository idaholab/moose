//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSplitCHWResAniso.h"

registerMooseObject("PhaseFieldApp", ADSplitCHWResAniso);

InputParameters
ADSplitCHWResAniso::validParams()
{
  InputParameters params = ADSplitCHWResBase<RealTensorValue>::validParams();
  params.addClassDescription("Split formulation Cahn-Hilliard Kernel for the chemical potential "
                             "variable with a scalar (isotropic) mobility");
  return params;
}

ADSplitCHWResAniso::ADSplitCHWResAniso(const InputParameters & parameters)
  : ADSplitCHWResBase<RealTensorValue>(parameters)
{
}
