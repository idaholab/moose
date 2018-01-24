/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SwitchingFunctionConstraintEta.h"

template <>
InputParameters
validParams<SwitchingFunctionConstraintEta>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Lagrange multiplier kernel to constrain the sum of all switching "
                             "functions in a multiphase system. This kernel acts on a "
                             "non-conserved order parameter eta_i.");
  params.addParam<MaterialPropertyName>("h_name",
                                        "Switching Function Materials that provides h(eta_i)");
  params.addRequiredCoupledVar("lambda", "Lagrange multiplier");
  return params;
}

SwitchingFunctionConstraintEta::SwitchingFunctionConstraintEta(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _eta_name(_var.name()),
    _dh(getMaterialPropertyDerivative<Real>("h_name", _eta_name)),
    _d2h(getMaterialPropertyDerivative<Real>("h_name", _eta_name, _eta_name)),
    _lambda(coupledValue("lambda")),
    _lambda_var(coupled("lambda"))
{
}

Real
SwitchingFunctionConstraintEta::computeQpResidual()
{
  return _lambda[_qp] * _dh[_qp] * _test[_i][_qp];
}

Real
SwitchingFunctionConstraintEta::computeQpJacobian()
{
  return _lambda[_qp] * _d2h[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
SwitchingFunctionConstraintEta::computeQpOffDiagJacobian(unsigned int j_var)
{
  if (j_var == _lambda_var)
    return _phi[_j][_qp] * _dh[_qp] * _test[_i][_qp];
  else
    return 0.0;
}
