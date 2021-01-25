//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticityKalidindiUpdate.h"

registerMooseObject("TensorMechanicsApp", CrystalPlasticityKalidindiUpdate);

InputParameters
CrystalPlasticityKalidindiUpdate::validParams()
{
  InputParameters params = CrystalPlasticityUpdate::validParams();
  params.addClassDescription("Kalidindi version of homogeneous crystal plasticity.");
  params.addParam<Real>("r", 1.0, "Latent hardening coefficient");
  params.addParam<Real>("h", 541.5, "hardening constants");
  params.addParam<Real>("t_sat", 109.8, "saturated slip system strength");
  params.addParam<Real>("gss_a", 2.5, "coefficient for hardening");
  params.addParam<Real>("ao", 0.001, "slip rate coefficient");
  params.addParam<Real>("xm", 0.1, "exponent for slip rate");
  params.addParam<Real>("gss_initial", 60.8, "initial lattice friction strength of the material");

  return params;
}

CrystalPlasticityKalidindiUpdate::CrystalPlasticityKalidindiUpdate(
    const InputParameters & parameters)
  : CrystalPlasticityUpdate(parameters),
    _slip_system_resistance(declareProperty<std::vector<Real>>("slip_system_resistance")),
    _slip_system_resistance_old(
        getMaterialPropertyOld<std::vector<Real>>("slip_system_resistance")),
    _slip_increment(declareProperty<std::vector<Real>>("plastic_slip_increment")),

    // Constitutive values
    _r(getParam<Real>("r")),
    _h(getParam<Real>("h")),
    _tau_sat(getParam<Real>("t_sat")),
    _gss_a(getParam<Real>("gss_a")),
    _ao(getParam<Real>("ao")),
    _xm(getParam<Real>("xm")),
    _gss_initial(getParam<Real>("gss_initial"))
{
  // resize local caching vectors used for substepping
  _previous_substep_slip_resistance.resize(_number_slip_systems);
  _slip_resistance_before_update.resize(_number_slip_systems);

  // resize vectors used in the consititutive slip hardening
  _hb.resize(_number_slip_systems);
  _slip_system_resistance_increment.resize(_number_slip_systems);
}

void
CrystalPlasticityKalidindiUpdate::initQpStatefulProperties()
{
  _slip_system_resistance[_qp].resize(_number_slip_systems);
  _slip_increment[_qp].resize(_number_slip_systems);

  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    _slip_system_resistance[_qp][i] = _gss_initial;
    _slip_increment[_qp][i] = 0.0;
  }

  CrystalPlasticityUpdate::initQpStatefulProperties();
}

void
CrystalPlasticityKalidindiUpdate::setInitialConstitutiveVariableValues()
{
  // Would also set old dislocation densities here if included in this model
  _slip_system_resistance[_qp] = _slip_system_resistance_old[_qp];
  _previous_substep_slip_resistance = _slip_system_resistance_old[_qp];
}

void
CrystalPlasticityKalidindiUpdate::setSubstepConstitutiveVariableValues()
{
  // Would also set substepped dislocation densities here if included in this model
  _slip_system_resistance[_qp] = _previous_substep_slip_resistance;
}

void
CrystalPlasticityKalidindiUpdate::calculateConstitutiveEquivalentSlipIncrement(
    RankTwoTensor & equivalent_slip_increment, bool & error_tolerance)
{
  equivalent_slip_increment.zero();

  if (_error_tolerance)
    return;

  // Would calculate the dislocation densities here if included in this model

  // Calculate the slip increment on each slip system
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    _slip_increment[_qp][i] =
        _ao * std::pow(std::abs(_tau[_qp][i] / _slip_system_resistance[_qp][i]), 1.0 / _xm);
    if (_tau[_qp][i] < 0.0)
      _slip_increment[_qp][i] *= -1.0;

    if (std::abs(_slip_increment[_qp][i]) * _substep_dt > _slip_incr_tol)
    {
#ifdef DEBUG
      mooseWarning("Maximum allowable slip increment exceeded ",
                   std::abs(_slip_increment[_qp][i]) * _substep_dt);
#endif
      error_tolerance = true;
      return;
    }
  }

  // Sum up the slip increments to find the equivalent plastic strain due to slip
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
    equivalent_slip_increment += _flow_direction[_qp][i] * _slip_increment[_qp][i] * _substep_dt;

  error_tolerance = false;
}

void
CrystalPlasticityKalidindiUpdate::calculateConstitutiveSlipDerivative(
    std::vector<Real> & dslip_dtau, unsigned int /*slip_model_number*/)
{
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    if (MooseUtils::absoluteFuzzyEqual(_tau[_qp][i], 0.0))
      dslip_dtau[i] = 0.0;
    else
      dslip_dtau[i] =
          _ao / _xm *
          std::pow(std::abs(_tau[_qp][i] / _slip_system_resistance[_qp][i]), 1.0 / _xm - 1.0) /
          _slip_system_resistance[_qp][i];
  }
}

void
CrystalPlasticityKalidindiUpdate::updateConstitutiveSlipSystemResistanceAndVariables(
    bool & error_tolerance)
{
  // Cache the slip system value before the update for the diff in the convergence check
  _slip_resistance_before_update = _slip_system_resistance[_qp];

  calculateSlipSystemResistance(error_tolerance);
}

bool
CrystalPlasticityKalidindiUpdate::areConstitutiveStateVariablesConverged()
{
  Real resistance_diff = 0.0;
  Real abs_prev_substep_resistance = 0.0;
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    resistance_diff = std::abs(_slip_resistance_before_update[i] - _slip_system_resistance[_qp][i]);
    abs_prev_substep_resistance = std::abs(_previous_substep_slip_resistance[i]);

    if (abs_prev_substep_resistance < _zero_tol && resistance_diff > _zero_tol)
      return true;
    else if (abs_prev_substep_resistance > _zero_tol &&
             resistance_diff > _resistance_tol * abs_prev_substep_resistance)
      return true;
  }

  return false;
}

void
CrystalPlasticityKalidindiUpdate::updateSubstepConstitutiveVariableValues()
{
  // Would also set substepped dislocation densities here if included in this model
  _previous_substep_slip_resistance = _slip_system_resistance[_qp];
}

void
CrystalPlasticityKalidindiUpdate::calculateSlipSystemResistance(bool & error_tolerance)
{
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    // Clear out increment from the previous iteration
    _slip_system_resistance_increment[i] = 0.0;

    _hb[i] = _h * std::pow(std::abs(1.0 - _slip_system_resistance[_qp][i] / _tau_sat), _gss_a);
    const Real hsign = 1.0 - _slip_system_resistance[_qp][i] / _tau_sat;
    if (hsign < 0.0)
      _hb[i] *= -1.0;
  }

  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    for (unsigned int j = 0; j < _number_slip_systems; ++j)
    {
      unsigned int iplane, jplane;
      iplane = i / 3;
      jplane = j / 3;

      if (iplane == jplane) // self vs. latent hardening
        _slip_system_resistance_increment[i] +=
            std::abs(_slip_increment[_qp][j]) * _hb[j]; // q_{ab} = 1.0 for self hardening
      else
        _slip_system_resistance_increment[i] +=
            std::abs(_slip_increment[_qp][j]) * _r * _hb[j]; // latent hardenign
    }
  }

  // Now perform the check to see if the slip system should be updated
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    _slip_system_resistance_increment[i] *= _substep_dt;
    if (_previous_substep_slip_resistance[i] < _zero_tol &&
        _slip_system_resistance_increment[i] < 0.0)
      _slip_system_resistance[_qp][i] = _previous_substep_slip_resistance[i];
    else
      _slip_system_resistance[_qp][i] =
          _previous_substep_slip_resistance[i] + _slip_system_resistance_increment[i];

    if (_slip_system_resistance[_qp][i] < 0.0)
      error_tolerance = true;
  }
}
