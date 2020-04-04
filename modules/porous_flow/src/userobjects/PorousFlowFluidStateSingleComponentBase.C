//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidStateSingleComponentBase.h"

InputParameters
PorousFlowFluidStateSingleComponentBase::validParams()
{
  InputParameters params = PorousFlowFluidStateBase::validParams();
  params.addClassDescription("Base class for single component fluid state classes");
  return params;
}

PorousFlowFluidStateSingleComponentBase::PorousFlowFluidStateSingleComponentBase(
    const InputParameters & parameters)
  : PorousFlowFluidStateBase(parameters), _pidx(0), _hidx(1), dT(1.0e-6)
{
}
