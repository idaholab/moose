/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TwoPhaseStressMaterial.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

template <>
InputParameters
validParams<TwoPhaseStressMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Compute a global stress in a two phase model");
  params.addParam<MaterialPropertyName>(
      "h", "h", "Switching Function Material that provides h(eta)");
  params.addRequiredParam<std::string>("base_A", "Base name for the Phase A strain.");
  params.addRequiredParam<std::string>("base_B", "Base name for the Phase B strain.");
  params.addParam<std::string>("base_name", "Base name for the computed global stress (optional).");
  return params;
}

TwoPhaseStressMaterial::TwoPhaseStressMaterial(const InputParameters & parameters)
  : Material(parameters),
    _h_eta(getMaterialProperty<Real>("h")),

    _base_A(getParam<std::string>("base_A") + "_"),
    _stress_A(getMaterialPropertyByName<RankTwoTensor>(_base_A + "stress")),
    _dstress_dstrain_A(getMaterialPropertyByName<RankFourTensor>(_base_A + "Jacobian_mult")),

    _base_B(getParam<std::string>("base_B") + "_"),
    _stress_B(getMaterialPropertyByName<RankTwoTensor>(_base_B + "stress")),
    _dstress_dstrain_B(getMaterialPropertyByName<RankFourTensor>(_base_B + "Jacobian_mult")),

    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(declareProperty<RankTwoTensor>(_base_name + "stress")),
    _dstress_dstrain(declareProperty<RankFourTensor>(_base_name + "Jacobian_mult"))
{
}

void
TwoPhaseStressMaterial::computeQpProperties()
{
  _stress[_qp] = _h_eta[_qp] * _stress_B[_qp] + (1.0 - _h_eta[_qp]) * _stress_A[_qp];
  _dstress_dstrain[_qp] =
      _h_eta[_qp] * _dstress_dstrain_B[_qp] + (1.0 - _h_eta[_qp]) * _dstress_dstrain_A[_qp];
}
