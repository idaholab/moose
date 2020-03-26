//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSplitCHWRes.h"

registerMooseObject("PhaseFieldApp", ADSplitCHWRes);

InputParameters
ADSplitCHWRes::validParams()
{
  InputParameters params = ADSplitCHWResBase<Real>::validParams();
  params.addClassDescription("Split formulation Cahn-Hilliard Kernel for the chemical potential "
                             "variable with a scalar (isotropic) mobility");
  return params;
}

ADSplitCHWRes::ADSplitCHWRes(const InputParameters & parameters)
  : ADSplitCHWResBase<Real>(parameters)
{
}
