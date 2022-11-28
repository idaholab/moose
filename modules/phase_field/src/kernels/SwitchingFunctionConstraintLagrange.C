//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SwitchingFunctionConstraintLagrange.h"

registerMooseObject("PhaseFieldApp", SwitchingFunctionConstraintLagrange);

InputParameters
SwitchingFunctionConstraintLagrange::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Lagrange multiplier kernel to constrain the sum of all switching "
                             "functions in a multiphase system. This kernel acts on the Lagrange "
                             "multiplier variable.");
  params.addParam<std::vector<MaterialPropertyName>>("h_names", "Switching function materials");
  params.addRequiredCoupledVar("etas", "eta order parameters");
  params.addParam<Real>("epsilon", 1e-9, "Shift factor to avoid a zero pivot");
  return params;
}

SwitchingFunctionConstraintLagrange::SwitchingFunctionConstraintLagrange(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _h_names(getParam<std::vector<MaterialPropertyName>>("h_names")),
    _num_h(_h_names.size()),
    _h(_num_h),
    _dh(_num_h),
    _eta_map(getParameterJvarMap("etas")),
    _epsilon(getParam<Real>("epsilon"))
{
  // parameter check. We need exactly one eta per h
  if (_num_h != coupledComponents("etas"))
    paramError("etas", "Need to pass in as many etas as h_names");

  // fetch switching functions (for the residual) and h derivatives (for the Jacobian)
  for (std::size_t i = 0; i < _num_h; ++i)
  {
    _h[i] = &getMaterialPropertyByName<Real>(_h_names[i]);

    _dh[i].resize(_num_h);
    for (std::size_t j = 0; j < _num_h; ++j)
      _dh[i][j] = &getMaterialPropertyDerivative<Real>(_h_names[i], coupledName("etas", j));
  }
}

Real
SwitchingFunctionConstraintLagrange::computeQpResidual()
{
  Real g = -_epsilon * _u[_qp] - 1.0;
  for (std::size_t i = 0; i < _num_h; ++i)
    g += (*_h[i])[_qp];

  return _test[_i][_qp] * g;
}

Real
SwitchingFunctionConstraintLagrange::computeQpJacobian()
{
  return _test[_i][_qp] * -_epsilon * _phi[_j][_qp];
}

Real
SwitchingFunctionConstraintLagrange::computeQpOffDiagJacobian(unsigned int jvar)
{
  auto eta = mapJvarToCvar(jvar, _eta_map);
  if (eta >= 0)
  {
    Real g = 0.0;
    for (std::size_t i = 0; i < _num_h; ++i)
      g += (*_dh[i][eta])[_qp] * _phi[_j][_qp];
    return g * _test[_i][_qp];
  }

  return 0.0;
}
