//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatchedScalarValueBC.h"
#include "Function.h"

#include "libmesh/node.h"

registerMooseObject("MooseTestApp", ADMatchedScalarValueBC);

InputParameters
ADMatchedScalarValueBC::validParams()
{
  InputParameters params = ADNodalBC::validParams();
  params.addRequiredCoupledVar("v", "The scalar variable to match");
  return params;
}

ADMatchedScalarValueBC::ADMatchedScalarValueBC(const InputParameters & parameters)
  : ADNodalBC(parameters), _v(adCoupledScalarValue("v"))
{
}

ADReal
ADMatchedScalarValueBC::computeQpResidual()
{
  return _u - _v[0];
}
