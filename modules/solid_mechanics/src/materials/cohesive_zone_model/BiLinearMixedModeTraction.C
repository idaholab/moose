//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BiLinearMixedModeTraction.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", BiLinearMixedModeTraction);

InputParameters
BiLinearMixedModeTraction::validParams()
{
  InputParameters params = CZMComputeLocalTractionTotalBase::validParams();
  params.addClassDescription("Mixed mode bilinear traction separation law.");
  params.addRequiredParam<Real>("penalty_stiffness", "Penalty stiffness.");
  params.addRequiredParam<MaterialPropertyName>(
      "GI_c", "Critical energy release rate in normal direction.");
  params.addRequiredParam<MaterialPropertyName>("GII_c",
                                                "Critical energy release rate in shear direction.");
  params.addRequiredParam<MaterialPropertyName>("normal_strength",
                                                "Tensile strength in normal direction.");
  params.addRequiredParam<MaterialPropertyName>("shear_strength",
                                                "Tensile strength in shear direction.");
  params.addRequiredParam<Real>("eta", "The power law parameter.");
  MooseEnum criterion("POWER_LAW BK", "BK");
  params.addParam<Real>("viscosity", 0.0, "Viscosity.");
  params.addParam<MooseEnum>(
      "mixed_mode_criterion", criterion, "Option for mixed mode propagation criterion.");

  // Advanced parameters to improve numerical convergence
  params.addParam<bool>(
      "lag_mode_mixity", true, "Whether to use old displacement jumps to compute the mode mixity.");
  params.addParam<bool>(
      "lag_displacement_jump",
      false,
      "Whether to use old displacement jumps to compute the effective displacement jump.");
  params.addParam<Real>("alpha", 1e-10, "Regularization parameter for the Macaulay bracket.");
  params.addParamNamesToGroup("lag_mode_mixity lag_displacement_jump alpha", "Advanced");

  return params;
}

BiLinearMixedModeTraction::BiLinearMixedModeTraction(const InputParameters & parameters)
  : CZMComputeLocalTractionTotalBase(parameters),
    _K(getParam<Real>("penalty_stiffness")),
    _d(declareProperty<Real>(_base_name + "damage")),
    _d_old(getMaterialPropertyOld<Real>(_base_name + "damage")),
    _interface_displacement_jump_old(
        getMaterialPropertyOld<RealVectorValue>(_base_name + "interface_displacement_jump")),
    _delta_init(
        declareProperty<Real>(_base_name + "effective_displacement_jump_at_damage_initiation")),
    _delta_final(
        declareProperty<Real>(_base_name + "effective_displacement_jump_at_full_degradation")),
    _delta_m(declareProperty<Real>(_base_name + "effective_displacement_jump")),
    _GI_c(getMaterialProperty<Real>(_base_name + getParam<MaterialPropertyName>("GI_c"))),
    _GII_c(getMaterialProperty<Real>(_base_name + getParam<MaterialPropertyName>("GII_c"))),
    _N(getMaterialProperty<Real>(_base_name + getParam<MaterialPropertyName>("normal_strength"))),
    _S(getMaterialProperty<Real>(_base_name + getParam<MaterialPropertyName>("shear_strength"))),
    _eta(getParam<Real>("eta")),
    _beta(declareProperty<Real>(_base_name + "mode_mixity_ratio")),
    _viscosity(getParam<Real>("viscosity")),
    _criterion(getParam<MooseEnum>("mixed_mode_criterion").getEnum<MixedModeCriterion>()),
    _lag_mode_mixity(getParam<bool>("lag_mode_mixity")),
    _lag_disp_jump(getParam<bool>("lag_displacement_jump")),
    _alpha(getParam<Real>("alpha"))
{
}

void
BiLinearMixedModeTraction::initQpStatefulProperties()
{
  CZMComputeLocalTractionTotalBase::initQpStatefulProperties();

  _d[_qp] = 0.0;
}

void
BiLinearMixedModeTraction::computeInterfaceTractionAndDerivatives()
{
  _interface_traction[_qp] = computeTraction();
  _dinterface_traction_djump[_qp] = computeTractionDerivatives();
}

RealVectorValue
BiLinearMixedModeTraction::computeTraction()
{
  computeModeMixity();
  computeCriticalDisplacementJump();
  computeFinalDisplacementJump();
  computeEffectiveDisplacementJump();
  computeDamage();

  // Split displacement jump into active and inactive parts
  const RealVectorValue delta = _interface_displacement_jump[_qp];
  const RealVectorValue delta_active(std::max(delta(0), 0.), delta(1), delta(2));
  const RealVectorValue delta_inactive(std::min(delta(0), 0.), 0, 0);

  return (1 - _d[_qp]) * _K * delta_active + _K * delta_inactive;
}

RankTwoTensor
BiLinearMixedModeTraction::computeTractionDerivatives()
{
  // The displacement jump split depends on the displacement jump, obviously
  const RealVectorValue delta = _interface_displacement_jump[_qp];
  RankTwoTensor ddelta_active_ddelta, ddelta_inactive_ddelta;
  ddelta_active_ddelta.fillFromInputVector({delta(0) > 0 ? 1. : 0., 1., 1.});
  ddelta_inactive_ddelta.fillFromInputVector({delta(0) < 0 ? 1. : 0., 0., 0.});

  RankTwoTensor dtraction_ddelta =
      (1 - _d[_qp]) * _K * ddelta_active_ddelta + _K * ddelta_inactive_ddelta;

  // The damage may also depend on the displacement jump
  const RealVectorValue delta_active(std::max(delta(0), 0.), delta(1), delta(2));
  RankTwoTensor A;
  A.vectorOuterProduct(delta_active, _dd_ddelta);
  dtraction_ddelta -= _K * A;

  return dtraction_ddelta;
}

void
BiLinearMixedModeTraction::computeModeMixity()
{
  const RealVectorValue delta =
      _lag_mode_mixity ? _interface_displacement_jump_old[_qp] : _interface_displacement_jump[_qp];

  if (delta(0) > 0)
  {
    const Real delta_s = std::sqrt(delta(1) * delta(1) + delta(2) * delta(2));
    _beta[_qp] = delta_s / delta(0);

    if (!_lag_mode_mixity)
      _dbeta_ddelta = RealVectorValue(-delta_s / delta(0) / delta(0),
                                      delta(1) / delta_s / delta(0),
                                      delta(2) / delta_s / delta(0));
  }
  else
  {
    _beta[_qp] = 0;
    _dbeta_ddelta = RealVectorValue(0, 0, 0);
  }
}

void
BiLinearMixedModeTraction::computeCriticalDisplacementJump()
{
  const RealVectorValue delta =
      _lag_mode_mixity ? _interface_displacement_jump_old[_qp] : _interface_displacement_jump[_qp];

  const Real delta_normal0 = _N[_qp] / _K;
  const Real delta_shear0 = _S[_qp] / _K;

  _delta_init[_qp] = delta_shear0;
  _ddelta_init_ddelta = RealVectorValue(0, 0, 0);
  if (delta(0) > 0)
  {
    const Real delta_mixed =
        std::sqrt(delta_shear0 * delta_shear0 + Utility::pow<2>(_beta[_qp] * delta_normal0));
    _delta_init[_qp] =
        delta_normal0 * delta_shear0 * std::sqrt(1 + _beta[_qp] * _beta[_qp]) / delta_mixed;
    if (!_lag_mode_mixity)
    {
      const Real ddelta_init_dbeta =
          _delta_init[_qp] * _beta[_qp] *
          (1 / (1 + _beta[_qp] * _beta[_qp]) - Utility::pow<2>(_delta_init[_qp] / delta_mixed));
      _ddelta_init_ddelta = ddelta_init_dbeta * _dbeta_ddelta;
    }
  }
}

void
BiLinearMixedModeTraction::computeFinalDisplacementJump()
{
  const RealVectorValue delta =
      _lag_mode_mixity ? _interface_displacement_jump_old[_qp] : _interface_displacement_jump[_qp];

  _delta_final[_qp] = std::sqrt(2) * 2 * _GII_c[_qp] / _S[_qp];
  _ddelta_final_ddelta = RealVectorValue(0, 0, 0);
  if (delta(0) > 0)
  {
    if (_criterion == MixedModeCriterion::BK)
    {
      _delta_final[_qp] =
          2 / _K / _delta_init[_qp] *
          (_GI_c[_qp] +
           (_GII_c[_qp] - _GI_c[_qp]) *
               std::pow(_beta[_qp] * _beta[_qp] / (1 + _beta[_qp] * _beta[_qp]), _eta));
      if (!_lag_mode_mixity)
      {
        const Real ddelta_final_ddelta_init = -_delta_final[_qp] / _delta_init[_qp];
        const Real ddelta_final_dbeta =
            2 / _K / _delta_init[_qp] * (_GII_c[_qp] - _GI_c[_qp]) * _eta *
            std::pow(_beta[_qp] * _beta[_qp] / (1 + _beta[_qp] * _beta[_qp]), _eta - 1) * 2 *
            _beta[_qp] * (1 - Utility::pow<2>(_beta[_qp] / (1 + _beta[_qp] * _beta[_qp])));
        _ddelta_final_ddelta =
            ddelta_final_ddelta_init * _ddelta_init_ddelta + ddelta_final_dbeta * _dbeta_ddelta;
      }
    }
    else if (_criterion == MixedModeCriterion::POWER_LAW)
    {
      const Real Gc_mixed =
          std::pow(1 / _GI_c[_qp], _eta) + std::pow(_beta[_qp] * _beta[_qp] / _GII_c[_qp], _eta);
      _delta_final[_qp] =
          (2 + 2 * _beta[_qp] * _beta[_qp]) / _K / _delta_init[_qp] * std::pow(Gc_mixed, -1 / _eta);
      if (!_lag_mode_mixity)
      {
        const Real ddelta_final_ddelta_init = -_delta_final[_qp] / _delta_init[_qp];
        const Real ddelta_final_dbeta =
            _delta_final[_qp] * 2 * _beta[_qp] / (1 + _beta[_qp] * _beta[_qp]) -
            (2 + 2 * _beta[_qp] * _beta[_qp]) / _K / _delta_init[_qp] *
                std::pow(Gc_mixed, -1 / _eta - 1) *
                (std::pow(1 / _GI_c[_qp], _eta - 1) +
                 std::pow(_beta[_qp] * _beta[_qp] / _GII_c[_qp], _eta - 1) * 2 * _beta[_qp] /
                     _GII_c[_qp]);
        _ddelta_final_ddelta =
            ddelta_final_ddelta_init * _ddelta_init_ddelta + ddelta_final_dbeta * _dbeta_ddelta;
      }
    }
  }
}

void
BiLinearMixedModeTraction::computeEffectiveDisplacementJump()
{
  const RealVectorValue delta =
      _lag_disp_jump ? _interface_displacement_jump_old[_qp] : _interface_displacement_jump[_qp];

  const Real delta_normal_pos = MathUtils::regularizedHeavyside(delta(0), _alpha) * delta(0);
  _delta_m[_qp] = std::sqrt(Utility::pow<2>(delta(1)) + Utility::pow<2>(delta(2)) +
                            Utility::pow<2>(delta_normal_pos));
  _ddelta_m_ddelta = RealVectorValue(0, 0, 0);
  if (!_lag_disp_jump && !MooseUtils::absoluteFuzzyEqual(_delta_m[_qp], 0))
  {
    const Real ddelta_normal_pos_ddelta_normal =
        MathUtils::regularizedHeavysideDerivative(delta(0), 1e-6) * delta(0) +
        MathUtils::regularizedHeavyside(delta(0), _alpha);
    _ddelta_m_ddelta =
        RealVectorValue(delta_normal_pos * ddelta_normal_pos_ddelta_normal, delta(1), delta(2));
    _ddelta_m_ddelta /= _delta_m[_qp];
  }
}

void
BiLinearMixedModeTraction::computeDamage()
{
  if (_delta_m[_qp] < _delta_init[_qp])
    _d[_qp] = 0;
  else if (_delta_m[_qp] > _delta_final[_qp])
    _d[_qp] = 1;
  else
    _d[_qp] = _delta_final[_qp] * (_delta_m[_qp] - _delta_init[_qp]) / _delta_m[_qp] /
              (_delta_final[_qp] - _delta_init[_qp]);

  _dd_ddelta = RealVectorValue(0, 0, 0);
  if (_d[_qp] < _d_old[_qp])
    // Irreversibility
    _d[_qp] = _d_old[_qp];
  else if (_delta_m[_qp] >= _delta_init[_qp] && _delta_m[_qp] <= _delta_final[_qp])
    _dd_ddelta = (_ddelta_final_ddelta * (_delta_m[_qp] - _delta_init[_qp]) +
                  _delta_final[_qp] * (_ddelta_m_ddelta - _ddelta_init_ddelta)) /
                     _delta_m[_qp] / (_delta_final[_qp] - _delta_init[_qp]) -
                 _delta_final[_qp] * (_delta_m[_qp] - _delta_init[_qp]) *
                     (_ddelta_m_ddelta * (_delta_final[_qp] - _delta_init[_qp]) +
                      _delta_m[_qp] * (_ddelta_final_ddelta - _ddelta_init_ddelta)) /
                     Utility::pow<2>(_delta_m[_qp] * (_delta_final[_qp] - _delta_init[_qp]));

  // Viscous regularization
  _d[_qp] = (_d[_qp] + _viscosity * _d_old[_qp] / _dt) / (_viscosity / _dt + 1);
  _dd_ddelta /= (_viscosity / _dt + 1);
}
