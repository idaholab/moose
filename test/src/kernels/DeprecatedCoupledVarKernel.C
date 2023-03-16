//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DeprecatedCoupledVarKernel.h"

registerMooseObject("MooseTestApp", DeprecatedCoupledVarKernel);

InputParameters
DeprecatedCoupledVarKernel::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addCoupledVar("stupid_name", "A variable representing a source term");
  params.deprecateCoupledVar("stupid_name", "source", "01/01/2040");
  return params;
}

DeprecatedCoupledVarKernel::DeprecatedCoupledVarKernel(const InputParameters & parameters)
  : ADKernel(parameters), _source(adCoupledValue("source"))
{
}

ADReal
DeprecatedCoupledVarKernel::computeQpResidual()
{
  return -_test[_i][_qp] * _source[_qp];
}
