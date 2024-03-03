//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Assembly.h"
#include "CZMComputeLocalTractionTotalBase.h"

InputParameters
CZMComputeLocalTractionTotalBase::validParams()
{
  InputParameters params = CZMComputeLocalTractionBase::validParams();

  params.addClassDescription("Base class for implementing cohesive zone constituive material "
                             "models that can be formulated using the total displacement jump");
  return params;
}

CZMComputeLocalTractionTotalBase::CZMComputeLocalTractionTotalBase(
    const InputParameters & parameters)
  : CZMComputeLocalTractionBase(parameters)
{
}
