//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DivField.h"
#include "Assembly.h"

registerMooseObject("MooseApp", DivField);

InputParameters
DivField::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("The divergence operator optionally scaled by a constant scalar "
                             "coefficient. Weak form: $(\\psi_i, k \\nabla \\cdot \\vec{u})$.");
  params.addRequiredCoupledVar("coupled_vector_variable", "The vector field");
  params.addParam<Real>("coeff", 1.0, "The constant coefficient");
  return params;
}

DivField::DivField(const InputParameters & parameters)
  : Kernel(parameters),
    _u_var(*getVectorVar("coupled_vector_variable", 0)),
    _u_var_num(coupled("coupled_vector_variable")),
    _div_u(coupledDiv("coupled_vector_variable")),
    _div_phi(_assembly.divPhi(_u_var)),
    _coeff(getParam<Real>("coeff"))
{
}

Real
DivField::computeQpResidual()
{
  return _coeff * _div_u[_qp] * _test[_i][_qp];
}

Real
DivField::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_u_var_num == jvar)
    return _coeff * _div_phi[_j][_qp] * _test[_i][_qp];

  return 0.0;
}
