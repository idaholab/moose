//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GradField.h"
#include "Assembly.h"

registerMooseObject("MooseApp", GradField);

InputParameters
GradField::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription("The gradient operator optionally scaled by a constant scalar "
                             "coefficient. Weak form: $(\\nabla \\cdot \\vec{\\psi_i}, k v)$.");
  params.addRequiredCoupledVar("coupled_scalar_variable", "The scalar field");
  params.addParam<Real>("coeff", 1.0, "The constant coefficient");
  return params;
}

GradField::GradField(const InputParameters & parameters)
  : VectorKernel(parameters),
    _p_var(*getVar("coupled_scalar_variable", 0)),
    _p_var_num(coupled("coupled_scalar_variable")),
    _p(coupledValue("coupled_scalar_variable")),
    _p_phi(_assembly.phi(_p_var)),
    _div_test(_var.divPhi()),
    _coeff(getParam<Real>("coeff"))
{
}

Real
GradField::computeQpResidual()
{
  return _coeff * _p[_qp] * _div_test[_i][_qp];
}

Real
GradField::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_p_var_num == jvar)
    return _coeff * _p_phi[_j][_qp] * _div_test[_i][_qp];

  return 0.0;
}
