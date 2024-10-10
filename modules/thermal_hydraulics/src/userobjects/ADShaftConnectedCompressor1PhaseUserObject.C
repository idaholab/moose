//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADShaftConnectedCompressor1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "VolumeJunction1Phase.h"
#include "MooseVariableScalar.h"
#include "THMIndicesVACE.h"
#include "Function.h"
#include "Numerics.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "libmesh/parallel_algebra.h"

registerMooseObject("ThermalHydraulicsApp", ADShaftConnectedCompressor1PhaseUserObject);

InputParameters
ADShaftConnectedCompressor1PhaseUserObject::validParams()
{
  InputParameters params = ADVolumeJunction1PhaseUserObject::validParams();
  params += ADShaftConnectableUserObjectInterface::validParams();

  params.addParam<BoundaryName>("inlet", "Compressor inlet");
  params.addParam<BoundaryName>("outlet", "Compressor outlet");
  params.addRequiredParam<Point>("di_out", "Direction of connected outlet");
  params.addRequiredParam<bool>("treat_as_turbine", "Treat the compressor as a turbine?");
  params.addRequiredParam<Real>("omega_rated", "Rated compressor speed [rad/s]");
  params.addRequiredParam<Real>("mdot_rated", "Rated compressor mass flow rate [kg/s]");
  params.addRequiredParam<Real>("rho0_rated",
                                "Rated compressor inlet stagnation fluid density [kg/m^3]");
  params.addRequiredParam<Real>("c0_rated", "Rated compressor inlet stagnation sound speed [m/s]");
  params.addRequiredParam<Real>("speed_cr_fr", "Compressor speed threshold for friction [-]");
  params.addRequiredParam<Real>("tau_fr_const", "Compressor friction constant [N-m]");
  params.addRequiredParam<std::vector<Real>>("tau_fr_coeff", "Friction coefficients [N-m]");
  params.addRequiredParam<Real>("speed_cr_I", "Compressor speed threshold for inertia [-]");
  params.addRequiredParam<Real>("inertia_const", "Compressor inertia constant [kg-m^2]");
  params.addRequiredParam<std::vector<Real>>("inertia_coeff",
                                             "Compressor inertia coefficients [kg-m^2]");
  params.addRequiredParam<std::vector<Real>>(
      "speeds",
      "Relative corrected speeds. Order of speeds needs correspond to the "
      "orders of `Rp_functions` and `eff_functions` [-]");
  params.addRequiredParam<std::vector<FunctionName>>(
      "Rp_functions",
      "Functions of pressure ratio versus relative corrected flow. Each function is for a "
      "different, constant relative corrected speed. The order of function names should correspond "
      "to the order of speeds in the `speeds` parameter [-]");
  params.addRequiredParam<std::vector<FunctionName>>(
      "eff_functions",
      "Functions of adiabatic efficiency versus relative corrected flow. Each function is for a "
      "different, constant relative corrected speed. The order of function names should correspond "
      "to the order of speeds in the `speeds` parameter [-]");
  params.addRequiredParam<Real>("min_pressure_ratio", "Minimum pressure ratio");
  params.addRequiredParam<Real>("max_pressure_ratio", "Maximum pressure ratio");
  params.addRequiredParam<std::string>("compressor_name",
                                       "Name of the instance of this compressor component");
  params.addRequiredCoupledVar("omega", "Shaft rotational speed [rad/s]");

  params.addClassDescription("Computes and caches flux and residual vectors for a 1-phase "
                             "compressor. Also computes compressor torque "
                             "and delta_p which is passed to the connected shaft");

  return params;
}

ADShaftConnectedCompressor1PhaseUserObject::ADShaftConnectedCompressor1PhaseUserObject(
    const InputParameters & params)
  : ADVolumeJunction1PhaseUserObject(params),
    ADShaftConnectableUserObjectInterface(this),

    _di_out(getParam<Point>("di_out")),
    _treat_as_turbine(getParam<bool>("treat_as_turbine")),
    _omega_rated(getParam<Real>("omega_rated")),
    _mdot_rated(getParam<Real>("mdot_rated")),
    _rho0_rated(getParam<Real>("rho0_rated")),
    _c0_rated(getParam<Real>("c0_rated")),
    _speed_cr_fr(getParam<Real>("speed_cr_fr")),
    _tau_fr_const(getParam<Real>("tau_fr_const")),
    _tau_fr_coeff(getParam<std::vector<Real>>("tau_fr_coeff")),
    _speed_cr_I(getParam<Real>("speed_cr_I")),
    _inertia_const(getParam<Real>("inertia_const")),
    _inertia_coeff(getParam<std::vector<Real>>("inertia_coeff")),
    _speeds(getParam<std::vector<Real>>("speeds")),
    _Rp_function_names(getParam<std::vector<FunctionName>>("Rp_functions")),
    _eff_function_names(getParam<std::vector<FunctionName>>("eff_functions")),
    _n_speeds(_speeds.size()),
    _Rp_functions(_n_speeds),
    _eff_functions(_n_speeds),
    _Rp_min(getParam<Real>("min_pressure_ratio")),
    _Rp_max(getParam<Real>("max_pressure_ratio")),
    _compressor_name(getParam<std::string>("compressor_name")),
    _omega(adCoupledScalarValue("omega"))
{
  if (_n_speeds != _Rp_function_names.size() || _n_speeds != _eff_function_names.size())
    mooseError("The number of entries of speeds needs to equal the number of entries of "
               "Rp_functions and eff_functions");

  // Store functions and check to make sure there is no self-reference.
  for (unsigned int i = 0; i < _n_speeds; i++)
  {
    if (_Rp_function_names[i] == name() || _eff_function_names[i] == name())
      mooseError(name(), ": This function cannot use its own name in the 'functions' parameter.");

    _Rp_functions[i] = &getFunctionByName(_Rp_function_names[i]);
    _eff_functions[i] = &getFunctionByName(_eff_function_names[i]);
  }
}

void
ADShaftConnectedCompressor1PhaseUserObject::initialSetup()
{
  ADVolumeJunction1PhaseUserObject::initialSetup();

  ADShaftConnectableUserObjectInterface::setupConnections(
      ADVolumeJunctionBaseUserObject::_n_connections, ADVolumeJunctionBaseUserObject::_n_flux_eq);
}

void
ADShaftConnectedCompressor1PhaseUserObject::initialize()
{
  ADVolumeJunction1PhaseUserObject::initialize();
  ADShaftConnectableUserObjectInterface::initialize();

  _isentropic_torque = 0;
  _dissipation_torque = 0;
  _friction_torque = 0;
  _delta_p = 0;
  _Rp = 0;
  _eff = 0;
  _flow_rel_corr = 0;
  _speed_rel_corr = 0;
}

void
ADShaftConnectedCompressor1PhaseUserObject::execute()
{
  ADVolumeJunctionBaseUserObject::storeConnectionData();
  ADShaftConnectableUserObjectInterface::setConnectionData(
      ADVolumeJunctionBaseUserObject::_flow_channel_dofs);

  const unsigned int c = getBoundaryIDIndex();

  computeFluxesAndResiduals(c);
}

void
ADShaftConnectedCompressor1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  ADVolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  // inlet c=0 established in component
  if (c == 0)
  {
    const ADReal rhoA_in = _rhoA[0];
    const ADReal rhouA_in = _rhouA[0];
    const ADReal rhoEA_in = _rhoEA[0];
    const ADReal e_in = THM::e_from_arhoA_arhouA_arhoEA(rhoA_in, rhouA_in, rhoEA_in);
    const ADReal v_in = _A[0] / rhoA_in;
    const ADReal p_in = _fp.p_from_v_e(v_in, e_in);
    const ADReal vel_in = rhouA_in / rhoA_in;

    // static entropy is equal to stagnation entropy by definition of the stagnation state
    const ADReal s_in = _fp.s_from_v_e(v_in, e_in);
    const ADReal s_out = s_in;

    // isentropic: dH/m = Vdp/m
    // h0, T0, and c0 are constant for adiabatic flows
    const ADReal h0_in = e_in + p_in * v_in + 0.5 * vel_in * vel_in;
    const ADReal p0_in = _fp.p_from_h_s(h0_in, s_in);
    const ADReal rho0_in = _fp.rho_from_p_s(p0_in, s_in);
    const ADReal v0_in = 1.0 / rho0_in;
    const ADReal e0_in = _fp.e_from_p_rho(p0_in, rho0_in);
    const ADReal c0_in = _fp.c_from_v_e(v0_in, e0_in);
    const ADReal flow_A = rhouA_in * _rho0_rated * _c0_rated;

    const ADReal flow_B = _mdot_rated * rho0_in * c0_in;
    _flow_rel_corr = flow_A / flow_B;
    _speed_rel_corr = (_omega[0] / _omega_rated) * (_c0_rated / c0_in);

    // If _speed_rel_corr == _speeds[0], then the lower index x1 is determined to be -1
    if (_speed_rel_corr == _speeds[0])
    {
      _Rp = _Rp_functions[0]->value(_flow_rel_corr, ADPoint());
      _eff = _eff_functions[0]->value(_flow_rel_corr, ADPoint());
    }
    else if (std::isnan(_speed_rel_corr)) // NaN; unguarded, gives segmentation fault
    {
      _Rp = std::nan("");
      _eff = std::nan("");
    }
    else // linear interpolation/extrapolation
    {
      unsigned int x1, x2;
      if (_speed_rel_corr < _speeds[0]) // extrapolation past minimum
      {
        x1 = 0;
        x2 = 1;
      }
      else if (_speed_rel_corr > _speeds.back()) // extrapolation past maximum
      {
        x1 = _n_speeds - 1;
        x2 = x1 - 1;
      }
      else // interpolation
      {
        auto x1_iter = std::lower_bound(_speeds.begin(), _speeds.end(), _speed_rel_corr);
        auto x2_iter = std::upper_bound(_speeds.begin(), _speeds.end(), _speed_rel_corr);

        x1 = (x1_iter - _speeds.begin()) - 1; // _speeds index for entry <= _speed_rel_corr
        x2 = (x2_iter - _speeds.begin());     // _speeds index for entry > _speed_rel_corr
      }

      const Real x1_spd = _speeds[x1];
      const Real x2_spd = _speeds[x2];

      const ADReal y1_Rp = _Rp_functions[x1]->value(_flow_rel_corr, ADPoint());
      const ADReal y2_Rp = _Rp_functions[x2]->value(_flow_rel_corr, ADPoint());
      const ADReal Rp_m = (y2_Rp - y1_Rp) / (x2_spd - x1_spd);
      _Rp = y1_Rp + (_speed_rel_corr - x1_spd) * Rp_m;

      const ADReal y1_eff = _eff_functions[x1]->value(_flow_rel_corr, ADPoint());
      const ADReal y2_eff = _eff_functions[x2]->value(_flow_rel_corr, ADPoint());
      const ADReal eff_m = (y2_eff - y1_eff) / (x2_spd - x1_spd);
      _eff = y1_eff + (_speed_rel_corr - x1_spd) * eff_m;
    }

    // Apply bounds
    _Rp = std::max(_Rp_min, std::min(_Rp_max, _Rp));

    // Invert if treating as turbine
    ADReal Rp_comp, eff_comp;
    if (_treat_as_turbine)
    {
      Rp_comp = 1.0 / _Rp;
      eff_comp = 1.0 / _eff;
    }
    else
    {
      Rp_comp = _Rp;
      eff_comp = _eff;
    }

    const ADReal p0_out = p0_in * Rp_comp;
    const ADReal rho0_out_isen = _fp.rho_from_p_s(p0_out, s_out);

    const ADReal e0_out_isen = _fp.e_from_p_rho(p0_out, rho0_out_isen);

    _delta_p = p0_in * (Rp_comp - 1.0);

    if (MooseUtils::absoluteFuzzyEqual(_omega[0], 0.0))
    {
      _isentropic_torque = 0.0;
      _dissipation_torque = 0.0;
    }
    else
    {
      const ADReal h0_out_isen = THM::h_from_e_p_rho(e0_out_isen, p0_out, rho0_out_isen);
      _isentropic_torque = -(rhouA_in / _omega[0]) * (h0_out_isen - h0_in); // tau_isen

      const ADReal g_x = h0_out_isen - h0_in + h0_in * eff_comp;
      const ADReal h0_out = g_x / eff_comp;

      _dissipation_torque = -(rhouA_in / _omega[0]) * (h0_out - h0_out_isen);
    }

    const ADReal alpha = _omega[0] / _omega_rated;

    if (alpha < _speed_cr_I)
    {
      _moment_of_inertia += _inertia_const;
    }
    else
    {
      _moment_of_inertia += _inertia_coeff[0] + _inertia_coeff[1] * std::abs(alpha) +
                            _inertia_coeff[2] * alpha * alpha +
                            _inertia_coeff[3] * std::abs(alpha * alpha * alpha);
    }

    // friction torque
    Real sign;
    if (_omega[0] >= 0)
      sign = -1;
    else
      sign = 1;
    if (alpha < _speed_cr_fr)
    {
      _friction_torque = sign * _tau_fr_const;
    }
    else
    {
      _friction_torque = sign * (_tau_fr_coeff[0] + _tau_fr_coeff[1] * std::abs(alpha) +
                                 _tau_fr_coeff[2] * alpha * alpha +
                                 _tau_fr_coeff[3] * std::abs(alpha * alpha * alpha));
    }

    _torque += _isentropic_torque + _dissipation_torque + _friction_torque;

    // compute momentum and energy source terms
    // a negative torque value results in a positive S_energy
    const ADReal S_energy = -(_isentropic_torque + _dissipation_torque) * _omega[0];

    const ADRealVectorValue S_momentum = _delta_p * _A_ref * _di_out;

    _residual[VolumeJunction1Phase::RHOEV_INDEX] -= S_energy;

    _residual[VolumeJunction1Phase::RHOUV_INDEX] -= S_momentum(0);
    _residual[VolumeJunction1Phase::RHOVV_INDEX] -= S_momentum(1);
    _residual[VolumeJunction1Phase::RHOWV_INDEX] -= S_momentum(2);
  }
}

ADReal
ADShaftConnectedCompressor1PhaseUserObject::getIsentropicTorque() const
{
  return _isentropic_torque;
}

ADReal
ADShaftConnectedCompressor1PhaseUserObject::getDissipationTorque() const
{
  return _dissipation_torque;
}

ADReal
ADShaftConnectedCompressor1PhaseUserObject::getFrictionTorque() const
{
  return _friction_torque;
}

ADReal
ADShaftConnectedCompressor1PhaseUserObject::getCompressorDeltaP() const
{
  return _delta_p;
}

ADReal
ADShaftConnectedCompressor1PhaseUserObject::getPressureRatio() const
{
  return _Rp;
}

ADReal
ADShaftConnectedCompressor1PhaseUserObject::getEfficiency() const
{
  return _eff;
}
ADReal
ADShaftConnectedCompressor1PhaseUserObject::getRelativeCorrectedMassFlowRate() const
{
  return _flow_rel_corr;
}
ADReal
ADShaftConnectedCompressor1PhaseUserObject::getRelativeCorrectedSpeed() const
{
  return _speed_rel_corr;
}

void
ADShaftConnectedCompressor1PhaseUserObject::finalize()
{
  ADVolumeJunction1PhaseUserObject::finalize();
  ADShaftConnectableUserObjectInterface::finalize();

  ADShaftConnectableUserObjectInterface::setupJunctionData(
      ADVolumeJunctionBaseUserObject::_scalar_dofs);
  ADShaftConnectableUserObjectInterface::setOmegaDofs(getScalarVar("omega", 0));

  comm().sum(_isentropic_torque);
  comm().sum(_dissipation_torque);
  comm().sum(_friction_torque);
  comm().sum(_delta_p);
  comm().sum(_Rp);
  comm().sum(_eff);
  comm().sum(_flow_rel_corr);
  comm().sum(_speed_rel_corr);
}

void
ADShaftConnectedCompressor1PhaseUserObject::threadJoin(const UserObject & uo)
{
  ADVolumeJunction1PhaseUserObject::threadJoin(uo);
  ADShaftConnectableUserObjectInterface::threadJoin(uo);

  const auto & scpuo = static_cast<const ADShaftConnectedCompressor1PhaseUserObject &>(uo);
  _isentropic_torque += scpuo._isentropic_torque;
  _dissipation_torque += scpuo._dissipation_torque;
  _friction_torque += scpuo._friction_torque;
  _delta_p += scpuo._delta_p;
  _Rp += scpuo._Rp;
  _eff += scpuo._eff;
  _flow_rel_corr += scpuo._flow_rel_corr;
  _speed_rel_corr += scpuo._speed_rel_corr;
}
