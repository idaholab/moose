//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatchedValueBC.h"

registerMooseObject("MooseApp", ADMatchedValueBC);

InputParameters
ADMatchedValueBC::validParams()
{
  InputParameters params = ADNodalBC::validParams();
  params.addRequiredCoupledVar("v", "The variable whose value we are to match.");
  params.addClassDescription("Implements a nodal BC which equates two different Variables' values "
                             "on a specified boundary.");
  return params;
}

ADMatchedValueBC::ADMatchedValueBC(const InputParameters & parameters)
  : ADNodalBC(parameters), _v(adCoupledValue("v"))
{
}

ADReal
ADMatchedValueBC::computeQpResidual()
{
  return _u - _v[_qp];
}
