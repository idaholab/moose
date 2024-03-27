//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVKernel.h"
#include "Assembly.h"
#include "SubProblem.h"

InputParameters
FVKernel::validParams()
{
  InputParameters params = ResidualObject::validParams();
  params += BlockRestrictable::validParams();
  params += ADFunctorInterface::validParams();
  params += FVRelationshipManagerInterface::validParams();

  params.registerBase("FVKernel");
  return params;
}

FVKernel::FVKernel(const InputParameters & params)
  : ResidualObject(params), BlockRestrictable(this), ADFunctorInterface(this)
{
  _subproblem.haveADObjects(true);
}
