//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ScalarCoupledPostprocessor.h"

registerMooseObject("MooseTestApp", ScalarCoupledPostprocessor);

InputParameters
ScalarCoupledPostprocessor::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar("variable", "Name of variable");
  params.addRequiredCoupledVar("coupled_scalar", "The name of the scalar coupled variable");
  return params;
}

ScalarCoupledPostprocessor::ScalarCoupledPostprocessor(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _coupled_scalar(coupledScalarValue("coupled_scalar")),
    _u(coupledValue("variable"))
{
}

Real
ScalarCoupledPostprocessor::computeQpIntegral()
{
  return _coupled_scalar[0] - _u[_qp];
}
