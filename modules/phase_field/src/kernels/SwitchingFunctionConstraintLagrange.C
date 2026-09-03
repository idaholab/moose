//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SwitchingFunctionConstraintLagrange.h"

registerMooseObject("PhaseFieldApp", SwitchingFunctionConstraintLagrange);
registerMooseObject("PhaseFieldApp", ADSwitchingFunctionConstraintLagrange);

template <bool is_ad>
InputParameters
SwitchingFunctionConstraintLagrangeTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addClassDescription("Lagrange multiplier kernel to constrain the sum of all switching "
                             "functions in a multiphase system. This kernel acts on the Lagrange "
                             "multiplier variable.");
  params.addParam<std::vector<MaterialPropertyName>>("h_names", "Switching function materials");
  params.addRequiredCoupledVar("etas", "eta order parameters");
  params.addParam<Real>("epsilon", 1e-9, "Shift factor to avoid a zero pivot");
  return params;
}

template <bool is_ad>
SwitchingFunctionConstraintLagrangeTempl<is_ad>::SwitchingFunctionConstraintLagrangeTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<GenericKernel<is_ad>>>(parameters),
    _h_names(this->template getParam<std::vector<MaterialPropertyName>>("h_names")),
    _num_h(_h_names.size()),
    _h(_num_h),
    _epsilon(this->template getParam<Real>("epsilon"))
{
  // parameter check. We need exactly one eta per h
  if (_num_h != coupledComponents("etas"))
    paramError("etas", "Need to pass in as many etas as h_names");

  // fetch switching functions (for the residual) and h derivatives (for the Jacobian)
  for (std::size_t i = 0; i < _num_h; ++i)
  {
    _h[i] = &this->template getGenericMaterialPropertyByName<Real, is_ad>(_h_names[i]);
  }
}

template <bool is_ad>
GenericReal<is_ad>
SwitchingFunctionConstraintLagrangeTempl<is_ad>::computeQpResidual()
{
  GenericReal<is_ad> g = -_epsilon * _u[_qp] - 1.0;
  for (std::size_t i = 0; i < _num_h; ++i)
    g += (*_h[i])[_qp];

  return _test[_i][_qp] * g;
}

SwitchingFunctionConstraintLagrange::SwitchingFunctionConstraintLagrange(
    const InputParameters & parameters)
  : SwitchingFunctionConstraintLagrangeTempl<false>(parameters),
    _dh(_num_h),
    _eta_map(getParameterJvarMap("etas"))
{
  for (std::size_t i = 0; i < _num_h; ++i)
  {
    _dh[i].resize(_num_h);
    for (std::size_t j = 0; j < _num_h; ++j)
      _dh[i][j] = &getMaterialPropertyDerivative<Real>(_h_names[i], coupledName("etas", j));
  }
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

template class SwitchingFunctionConstraintLagrangeTempl<false>;
template class SwitchingFunctionConstraintLagrangeTempl<true>;
