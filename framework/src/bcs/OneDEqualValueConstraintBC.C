/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "OneDEqualValueConstraintBC.h"

template <>
InputParameters
validParams<OneDEqualValueConstraintBC>()
{
  InputParameters params = validParams<IntegratedBC>();
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
OneDEqualValueConstraintBC::computeQpOffDiagJacobian(unsigned jvar)
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
