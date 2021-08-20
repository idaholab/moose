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
  params.addRequiredParam<Real>("eta", "The B-K power law parameter.");
  params.addCoupledVar("normal_strength_scale_factor", "Scale factor for normal strength.");
  params.addClassDescription("Mixed mode bilinear traction separation law.");
  return params;
}

BiLinearMixedModeTraction::BiLinearMixedModeTraction(const InputParameters & parameters)
  : CZMComputeLocalTractionTotalBase(parameters),
    _scale_factor(isCoupled("normal_strength_scale_factor")
                      ? &coupledValue("normal_strength_scale_factor")
                      : nullptr),
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
    _eff_disp_full_degradation(
        declareProperty<Real>(_base_name + "effective_displacement_at_full_degradation")),
    _eta(getParam<Real>("eta")),
    _maximum_mixed_mode_relative_displacement(
        declareProperty<Real>(_base_name + "maximum_mixed_mode_relative_displacement")),
    _maximum_mixed_mode_relative_displacement_old(
        getMaterialPropertyOld<Real>(_base_name + "maximum_mixed_mode_relative_displacement")),
    _beta(declareProperty<Real>("mode_mixity_ratio"))
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
}

RealVectorValue
BiLinearMixedModeTraction::computeTraction()
{
  _beta_sq = 0.0;

  Real delta_normal0 = _delta_normal0;

  if (_scale_factor != nullptr)
    delta_normal0 *= (*_scale_factor)[_qp];

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
    _eff_disp_full_degradation[_qp] =
        2.0 / (_stiffness * _eff_disp_damage_init[_qp]) *
        (_GI_C + (_GII_C - _GI_C) * std::pow(_beta_sq / (1 + _beta_sq), _eta));
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

  if (_maximum_mixed_mode_relative_displacement[_qp] <= _eff_disp_damage_init[_qp])
  {
    _D.addIa(_stiffness);
  }
  else if (_maximum_mixed_mode_relative_displacement[_qp] <= _eff_disp_full_degradation[_qp])
  {
    _D.addIa((1 - _d[_qp]) * _stiffness);
    Real term = _stiffness * _d[_qp] *
                (std::max(-_interface_displacement_jump[_qp](0), 0.0) /
                 (-_interface_displacement_jump[_qp](0)));
    _D(0, 0) += term;
    _D(0, 1) += term;
    _D(0, 2) += term;

    if (!use_old_state)
    {
      RealVectorValue dmax_du(
          _interface_displacement_jump[_qp](0) > 0.0
              ? _interface_displacement_jump[_qp](0) /
                    _maximum_mixed_mode_relative_displacement[_qp]
              : 0.0,
          _interface_displacement_jump[_qp](1) / _maximum_mixed_mode_relative_displacement[_qp],
          _interface_displacement_jump[_qp](2) / _maximum_mixed_mode_relative_displacement[_qp]);

      dmax_du *= _eff_disp_full_degradation[_qp] * _eff_disp_damage_init[_qp] /
                 (_eff_disp_full_degradation[_qp] - _eff_disp_damage_init[_qp]) /
                 Utility::pow<2>(_maximum_mixed_mode_relative_displacement[_qp]);

      if (_interface_displacement_jump[_qp](0) > 0.0)
        _dDdu.vectorOuterProduct(-1.0 * _stiffness * _interface_displacement_jump[_qp], dmax_du);
      else
        _dDdu.vectorOuterProduct(
            RealVectorValue(-1.0 * _stiffness * _interface_displacement_jump[_qp](0) +
                                _stiffness * (_interface_displacement_jump[_qp](0) +
                                              _interface_displacement_jump[_qp](1) +
                                              _interface_displacement_jump[_qp](2)),
                            -1.0 * _stiffness * _interface_displacement_jump[_qp](1),
                            -1.0 * _stiffness * _interface_displacement_jump[_qp](2)),
            dmax_du);
    }

    _dDdu *= (_dt / (_viscosity + _dt));
  }
  else
  {
    _D(0, 0) = _stiffness * (std::max(-_interface_displacement_jump[_qp](0), 0.0) /
                             (-_interface_displacement_jump[_qp](0)));
  }

  return _D * _interface_displacement_jump[_qp];
}

RankTwoTensor
BiLinearMixedModeTraction::computeTractionDerivatives()
{
  return _D + _dDdu;
}
