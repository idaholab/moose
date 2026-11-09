//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "ADVectorDivDiv.h"

registerMooseObject("MooseTestApp", ADVectorDivDiv);

InputParameters
ADVectorDivDiv::validParams()
{
  InputParameters params = ADVectorKernel::validParams();
  params.addClassDescription("The AD grad-div operator $(\\nabla \\cdot \\vec{\\psi_i}, k \\nabla "
                             "\\cdot \\vec{v})$, where $\\vec{v}$ is the variable or a coupled "
                             "vector variable.");
  params.addCoupledVar("v", "Vector variable whose divergence is used instead of the variable's");
  params.addParam<Real>("coeff", 1.0, "The constant coefficient");
  return params;
}

ADVectorDivDiv::ADVectorDivDiv(const InputParameters & parameters)
  : ADVectorKernel(parameters),
    _div_test(_var.divPhi()),
    _div_v(isCoupled("v") ? adCoupledDiv("v") : _var.adDivSln()),
    _coeff(getParam<Real>("coeff"))
{
}

ADReal
ADVectorDivDiv::computeQpResidual()
{
  return _coeff * _div_v[_qp] * _div_test[_i][_qp];
}
