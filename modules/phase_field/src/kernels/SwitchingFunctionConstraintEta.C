//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SwitchingFunctionConstraintEta.h"

registerMooseObject("PhaseFieldApp", SwitchingFunctionConstraintEta);

InputParameters
SwitchingFunctionConstraintEta::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Lagrange multiplier kernel to constrain the sum of all switching "
                             "functions in a multiphase system. This kernel acts on a "
                             "non-conserved order parameter eta_i.");
  params.addParam<MaterialPropertyName>("h_name",
                                        "Switching Function Materials that provides h(eta_i)");
  params.addRequiredCoupledVar("lambda", "Lagrange multiplier");
  params.addCoupledVar("args", "Vector of further variable arguments to the switching function");
  params.deprecateCoupledVar("args", "coupled_variables", "02/27/2024");
  return params;
}

SwitchingFunctionConstraintEta::SwitchingFunctionConstraintEta(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _eta_name(_var.name()),
    _dh(getMaterialPropertyDerivative<Real>("h_name", _eta_name)),
    _d2h(getMaterialPropertyDerivative<Real>("h_name", _eta_name, _eta_name)),
    _d2ha(isCoupled("args") ? coupledComponents("args") : coupledComponents("coupled_variables")),
    _d2ha_map(isCoupled("args") ? getParameterJvarMap("args")
                                : getParameterJvarMap("coupled_variables")),
    _lambda(coupledValue("lambda")),
    _lambda_var(coupled("lambda"))
{
  for (std::size_t i = 0; i < _d2ha.size(); ++i)
  {
    if (isCoupled("args"))
      _d2ha[i] = &getMaterialPropertyDerivative<Real>("h_name", _eta_name, coupledName("args", i));
    else
      _d2ha[i] = &getMaterialPropertyDerivative<Real>(
          "h_name", _eta_name, coupledName("coupled_variables", i));
  }
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
SwitchingFunctionConstraintEta::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _lambda_var)
    return _phi[_j][_qp] * _dh[_qp] * _test[_i][_qp];

  auto k = mapJvarToCvar(jvar, _d2ha_map);
  if (k >= 0)
    return _lambda[_qp] * (*_d2ha[k])[_qp] * _phi[_j][_qp] * _test[_i][_qp];

  return 0.0;
}
