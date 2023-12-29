//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCZMComputeLocalTractionTotalBase.h"
#include "CZMComputeLocalTractionTotalBase.h"

InputParameters
ADCZMComputeLocalTractionTotalBase::validParams()
{
  InputParameters params = CZMComputeLocalTractionTotalBase::validParams();
  return params;
}

ADCZMComputeLocalTractionTotalBase::ADCZMComputeLocalTractionTotalBase(
    const InputParameters & parameters)
  : ADCZMComputeLocalTractionBase(parameters)
{
}
