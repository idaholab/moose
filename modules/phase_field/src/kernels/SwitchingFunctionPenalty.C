//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SwitchingFunctionPenalty.h"

registerMooseObject("PhaseFieldApp", SwitchingFunctionPenalty);

InputParameters
SwitchingFunctionPenalty::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Penalty kernel to constrain the sum of all switching functions in a multiphase system.");
  params.addParam<std::vector<MaterialPropertyName>>(
      "h_names", "Switching Function Materials that provide h(eta_i)");
  params.addRequiredCoupledVar("etas", "eta_i order parameters, one for each h");
  params.addParam<Real>("penalty", 1.0, "Penalty scaling factor");
  return params;
}

SwitchingFunctionPenalty::SwitchingFunctionPenalty(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _h_names(getParam<std::vector<MaterialPropertyName>>("h_names")),
    _num_h(_h_names.size()),
    _h(_num_h),
    _dh(_num_h),
    _penalty(getParam<Real>("penalty")),
    _number_of_nl_variables(_fe_problem.getNonlinearSystemBase().nVariables()),
    _j_eta(_number_of_nl_variables, -1),
    _a(-1)
{
  // parameter check. We need exactly one eta per h
  if (_num_h != coupledComponents("etas"))
    paramError("h_names", "Need to pass in as many h_names as etas");

  // fetch switching functions (for the residual) and h derivatives (for the Jacobian)
  for (unsigned int i = 0; i < _num_h; ++i)
  {
    _h[i] = &getMaterialPropertyByName<Real>(_h_names[i]);
    _dh[i] = &getMaterialPropertyDerivative<Real>(_h_names[i], coupledName("etas", i));

    // generate the lookup table from j_var -> eta index
    unsigned int num = coupled("etas", i);
    if (num < _number_of_nl_variables)
      _j_eta[num] = i;

    // figure out which variable this kernel is acting on
    if (num == _var.number())
      _a = i;
  }

  if (_a < 0)
    paramError("etas", "Kernel variable must be listed in etas");

  _d2h = &getMaterialPropertyDerivative<Real>(_h_names[_a], _var.name(), _var.name());
}

Real
SwitchingFunctionPenalty::computeQpResidual()
{
  Real g = -1.0;
  for (unsigned int i = 0; i < _num_h; ++i)
    g += (*_h[i])[_qp];

  return _test[_i][_qp] * _penalty * 2.0 * g * (*_dh[_a])[_qp];
}

Real
SwitchingFunctionPenalty::computeQpJacobian()
{
  Real g = -1.0;
  for (unsigned int i = 0; i < _num_h; ++i)
    g += (*_h[i])[_qp];

  return _test[_i][_qp] * _penalty * _phi[_j][_qp] * 2.0 *
         ((*_dh[_a])[_qp] * (*_dh[_a])[_qp] + g * (*_d2h)[_qp]);
}

Real
SwitchingFunctionPenalty::computeQpOffDiagJacobian(unsigned int j_var)
{
  const int eta = _j_eta[j_var];

  if (eta >= 0)
    return _test[_i][_qp] * _penalty * _phi[_j][_qp] * 2.0 * (*_dh[eta])[_qp] * (*_dh[_a])[_qp];
  else
    return 0.0;
}
