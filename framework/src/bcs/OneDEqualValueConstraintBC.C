//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDEqualValueConstraintBC.h"

registerMooseObject("MooseApp", OneDEqualValueConstraintBC);

InputParameters
OneDEqualValueConstraintBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("Computes the integral of lambda times dg term from the mortar method"
                             " (for two 1D domains only).");
  params.addRequiredCoupledVar("lambda", "Lagrange multiplier");
  params.addRequiredParam<unsigned int>("component", "Component of the Lagrange multiplier");
  params.addRequiredParam<Real>(
      "vg",
      "Variation of the constraint g wrt this surface (+1 or -1). Note: g = value1 - value2 = 0 ");
  return params;
}

OneDEqualValueConstraintBC::OneDEqualValueConstraintBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _lambda(coupledScalarValue("lambda")),
    _lambda_var_number(coupledScalar("lambda")),
    _component(getParam<unsigned int>("component")),
    _vg(getParam<Real>("vg"))
{
}

Real
OneDEqualValueConstraintBC::computeQpResidual()
{
  return _lambda[_component] * _vg * _test[_i][_qp];
}

Real
OneDEqualValueConstraintBC::computeQpJacobian()
{
  return 0.;
}

Real
OneDEqualValueConstraintBC::computeQpOffDiagJacobianScalar(unsigned int jvar)
{
  if (jvar == _lambda_var_number)
  {
    if (_j == _component)
      return _vg * _test[_i][_qp];
    else
      return 0.;
  }
  else
    return 0.;
}
