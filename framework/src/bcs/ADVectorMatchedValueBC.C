//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVectorMatchedValueBC.h"

registerMooseObject("MooseApp", ADVectorMatchedValueBC);

InputParameters
ADVectorMatchedValueBC::validParams()
{
  InputParameters params = ADVectorNodalBC::validParams();
  params.addRequiredCoupledVar("v", "The variable whose value we are to match.");
  params.addClassDescription(
      "Implements a ADVectorNodalBC which equates two different Variables' values "
      "on a specified boundary.");
  return params;
}

ADVectorMatchedValueBC::ADVectorMatchedValueBC(const InputParameters & parameters)
  : ADVectorNodalBC(parameters), _v(adCoupledNodalValue<RealVectorValue>("v"))
{
}

ADRealVectorValue
ADVectorMatchedValueBC::computeQpResidual()
{
  return _u - _v;
}
