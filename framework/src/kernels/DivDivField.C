//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DivDivField.h"
#include "Assembly.h"

registerMooseObject("MooseApp", DivDivField);

InputParameters
DivDivField::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription(
      "The grad-div operator optionally scaled by a constant scalar coefficient."
      "Weak form: $(\\nabla \\cdot \\vec{\\psi_i}, k \\nabla \\cdot \\vec{u})$.");
  params.addParam<Real>("coeff", 1.0, "The constant coefficient");
  return params;
}

DivDivField::DivDivField(const InputParameters & parameters)
  : VectorKernel(parameters),
    _div_test(_var.divPhi()),
    _div_phi(_assembly.divPhi(_var)),
    _div_u(_is_implicit ? _var.divSln() : _var.divSlnOld()),
    _coeff(getParam<Real>("coeff"))
{
}

Real
DivDivField::computeQpResidual()
{
  return _coeff * _div_u[_qp] * _div_test[_i][_qp];
}

Real
DivDivField::computeQpJacobian()
{
  return _coeff * _div_phi[_j][_qp] * _div_test[_i][_qp];
}
