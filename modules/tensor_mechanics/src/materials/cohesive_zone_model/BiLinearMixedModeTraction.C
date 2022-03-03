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
  params.addRequiredParam<Real>("penalty_stiffness", "Penalty stiffness.");
  params.addRequiredParam<Real>("GI_C", "Critical energy release rate in normal direction.");
  params.addRequiredParam<Real>("GII_C", "Critical energy release rate in shear direction.");
  params.addParam<Real>("viscosity", 0.0, "Viscosity.");
  params.addRequiredParam<Real>("normal_strength", "Tensile strength in normal direction.");
  params.addRequiredParam<Real>("shear_strength", "Tensile strength in shear direction.");
  params.addRequiredParam<Real>("eta", "The power law parameter.");
  params.addParam<Real>("alpha", 1.0e-10, "The interpenetration regularization parameter.");
  params.addParam<bool>(
      "lag_seperation_state", false, "Lag seperation sate: use their old values.");
  MooseEnum criterion("POWER_LAW BK", "BK");
  params.addParam<MooseEnum>(
      "mixed_mode_criterion", criterion, "Option for mixed mode propagation criterion.");
  params.addCoupledVar("normal_strength_scale_factor", 1.0, "Scale factor for normal strength.");
  params.addClassDescription("Mixed mode bilinear traction separation law.");
  return params;
}

BiLinearMixedModeTraction::BiLinearMixedModeTraction(const InputParameters & parameters)
  : CZMComputeLocalTractionTotalBase(parameters),
    _scale_factor(coupledValue("normal_strength_scale_factor")),
    _stiffness(getParam<Real>("penalty_stiffness")),
    _d(declareProperty<Real>("damage")),
    _d_old(getMaterialPropertyOld<Real>("damage")),
    _interface_displacement_jump_old(
        getMaterialPropertyOld<RealVectorValue>("interface_displacement_jump")),
    _viscosity(getParam<Real>("viscosity")),
    _GI_C(getParam<Real>("GI_C")),
    _GII_C(getParam<Real>("GII_C")),
    _delta_normal0(getParam<Real>("normal_strength") / _stiffness),
    _delta_shear0(getParam<Real>("shear_strength") / _stiffness),
    _eff_disp_damage_init(
        declareProperty<Real>(_base_name + "effective_displacement_at_damage_initiation")),
    _eff_disp_damage_init_old(
        getMaterialPropertyOld<Real>(_base_name + "effective_displacement_at_damage_initiation")),
    _eff_disp_full_degradation(
        declareProperty<Real>(_base_name + "effective_displacement_at_full_degradation")),
    _eff_disp_full_degradation_old(
        getMaterialPropertyOld<Real>(_base_name + "effective_displacement_at_full_degradation")),
    _eta(getParam<Real>("eta")),
    _maximum_mixed_mode_relative_displacement(
        declareProperty<Real>(_base_name + "maximum_mixed_mode_relative_displacement")),
    _maximum_mixed_mode_relative_displacement_old(
        getMaterialPropertyOld<Real>(_base_name + "maximum_mixed_mode_relative_displacement")),
    _beta(declareProperty<Real>("mode_mixity_ratio")),
    _criterion(getParam<MooseEnum>("mixed_mode_criterion").getEnum<MixedModeCriterion>()),
    _lag_seperation_state(getParam<bool>("lag_seperation_state")),
    _alpha(getParam<Real>("alpha"))
{
}

void
BiLinearMixedModeTraction::computeInterfaceTractionAndDerivatives()
{
  _interface_traction[_qp] = computeTraction();
  _dinterface_traction_djump[_qp] = computeTractionDerivatives();
}

void
BiLinearMixedModeTraction::initQpStatefulProperties()
{
  CZMComputeLocalTractionTotalBase::initQpStatefulProperties();

  _d[_qp] = 0.0;
  _maximum_mixed_mode_relative_displacement[_qp] = 0.0;
  _eff_disp_damage_init[_qp] = 0.0;
  _eff_disp_full_degradation[_qp] = 0.0;
}

RealVectorValue
BiLinearMixedModeTraction::computeTraction()
{
  _beta_sq = 0.0;

  const Real delta_normal0 = _delta_normal0 * _scale_factor[_qp];

  const Real eff_shear_disp = std::sqrt(Utility::pow<2>(_interface_displacement_jump[_qp](1)) +
                                        Utility::pow<2>(_interface_displacement_jump[_qp](2)));

  // compute mode mixity ratio, effective displacement at damage initiation and effective
  // displacement at full degradation
  if (_interface_displacement_jump[_qp](0) > 0.0)
  {
    _beta_sq = Utility::pow<2>(eff_shear_disp / _interface_displacement_jump[_qp](0));
    _eff_disp_damage_init[_qp] =
        delta_normal0 * _delta_shear0 *
        std::sqrt((1 + _beta_sq) /
                  (Utility::pow<2>(_delta_shear0) + _beta_sq * Utility::pow<2>(delta_normal0)));

    switch (_criterion)
    {
      case MixedModeCriterion::BK:
      {
        _eff_disp_full_degradation[_qp] =
            2.0 / (_stiffness * _eff_disp_damage_init[_qp]) *
            (_GI_C + (_GII_C - _GI_C) * std::pow(_beta_sq / (1 + _beta_sq), _eta));
        break;
      }
      case MixedModeCriterion::POWER_LAW:
      {
        _eff_disp_full_degradation[_qp] =
            (2.0 + 2.0 * _beta_sq) / (_stiffness * _eff_disp_damage_init[_qp]) *
            std::pow(std::pow(1.0 / _GI_C, _eta) + std::pow(_beta_sq / _GII_C, _eta), -1.0 / _eta);
        break;
      }
    }
  }
  else
  {
    _beta_sq = 0.0;
    _eff_disp_damage_init[_qp] =
        std::sqrt(Utility::pow<2>(_delta_shear0) + Utility::pow<2>(delta_normal0));
    _eff_disp_full_degradation[_qp] =
        std::sqrt(2.0 * Utility::pow<2>(2 * _GII_C / (_delta_shear0 * _stiffness)));
  }

  _beta[_qp] = std::sqrt(_beta_sq);

  // compute mixed mode relative displacement
  Real mixed_mode_relative_displacement =
      std::sqrt(Utility::pow<2>(_interface_displacement_jump[_qp](1)) +
                Utility::pow<2>(_interface_displacement_jump[_qp](2)) +
                Utility::pow<2>(std::max(_interface_displacement_jump[_qp](0), 0.0)));

  bool use_old_state =
      (mixed_mode_relative_displacement < _maximum_mixed_mode_relative_displacement_old[_qp]) ? 1
                                                                                              : 0;

  _maximum_mixed_mode_relative_displacement[_qp] =
      use_old_state ? _maximum_mixed_mode_relative_displacement_old[_qp]
                    : mixed_mode_relative_displacement;

  /// compute damage variable
  Real d = 0.0;

  if (_maximum_mixed_mode_relative_displacement[_qp] *
          (_eff_disp_full_degradation[_qp] - _eff_disp_damage_init[_qp]) >
      0.0)
    d = _eff_disp_full_degradation[_qp] *
        (_maximum_mixed_mode_relative_displacement[_qp] - _eff_disp_damage_init[_qp]) /
        (_maximum_mixed_mode_relative_displacement[_qp] *
         (_eff_disp_full_degradation[_qp] - _eff_disp_damage_init[_qp]));

  d = MathUtils::clamp(d, 0.0, 1.0);

  // viscous regularization
  d = (d + _viscosity * _d_old[_qp] / _dt) / (_viscosity / _dt + 1.0);

  if (d > _d_old[_qp])
    _d[_qp] = d;
  else
  {
    _d[_qp] = _d_old[_qp];
    use_old_state = true;
  }

  _D.zero();
  _dDdu.zero();

  Real maximum_mixed_mode_relative_displacement = _maximum_mixed_mode_relative_displacement[_qp];
  Real eff_disp_damage_init = _eff_disp_damage_init[_qp];
  Real eff_disp_full_degradation = _eff_disp_full_degradation[_qp];
  RealVectorValue interface_displacement_jump = _interface_displacement_jump[_qp];

  if (_lag_seperation_state)
  {
    maximum_mixed_mode_relative_displacement = _maximum_mixed_mode_relative_displacement_old[_qp];
    eff_disp_damage_init = _eff_disp_damage_init_old[_qp];
    eff_disp_full_degradation = _eff_disp_full_degradation_old[_qp];
    interface_displacement_jump = _interface_displacement_jump_old[_qp];
  }

  if (maximum_mixed_mode_relative_displacement <= eff_disp_damage_init)
  {
    _D.addIa(_stiffness);
  }
  else if (maximum_mixed_mode_relative_displacement <= eff_disp_full_degradation)
  {
    _D.addIa((1 - _d[_qp]) * _stiffness);

    Real term = _stiffness * _d[_qp] *
                MathUtils::regularizedHeavyside(-interface_displacement_jump(0), _alpha);

    _D(0, 0) += term;
    _D(0, 1) += term;
    _D(0, 2) += term;

    if (!use_old_state && !_lag_seperation_state)
    {
      RealVectorValue dmax_du(
          interface_displacement_jump(0) > 0.0
              ? interface_displacement_jump(0) / maximum_mixed_mode_relative_displacement
              : 0.0,
          interface_displacement_jump(1) / maximum_mixed_mode_relative_displacement,
          interface_displacement_jump(2) / maximum_mixed_mode_relative_displacement);

      dmax_du *= eff_disp_full_degradation * eff_disp_damage_init /
                 (eff_disp_full_degradation - eff_disp_damage_init) /
                 Utility::pow<2>(maximum_mixed_mode_relative_displacement);

      _dDdu = RankTwoTensor::vectorOuterProduct(
          RealVectorValue(
              -1.0 * _stiffness * interface_displacement_jump(0) +
                  _stiffness *
                      MathUtils::regularizedHeavyside(-interface_displacement_jump(0), _alpha) *
                      (interface_displacement_jump(0) + interface_displacement_jump(1) +
                       interface_displacement_jump(2)),
              -1.0 * _stiffness * interface_displacement_jump(1),
              -1.0 * _stiffness * interface_displacement_jump(2)),
          dmax_du);

      _dDdu *= (_dt / (_viscosity + _dt));
    }
  }
  else if (maximum_mixed_mode_relative_displacement > eff_disp_full_degradation)
  {
    _D(0, 0) =
        _stiffness * MathUtils::regularizedHeavyside(-interface_displacement_jump(0), _alpha);

    if (!_lag_seperation_state)
      _dDdu(0, 0) =
          -_stiffness *
          MathUtils::regularizedHeavysideDerivative(-interface_displacement_jump(0), _alpha) *
          interface_displacement_jump(0);
  }

  return _D * _interface_displacement_jump[_qp];
}

RankTwoTensor
BiLinearMixedModeTraction::computeTractionDerivatives()
{
  return _D + _dDdu;
}
