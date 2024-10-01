//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVolumeJunction1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndices3Eqn.h"
#include "VolumeJunction1Phase.h"
#include "ADNumericalFlux3EqnBase.h"
#include "Numerics.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "libmesh/parallel_algebra.h"

registerMooseObject("ThermalHydraulicsApp", ADVolumeJunction1PhaseUserObject);

InputParameters
ADVolumeJunction1PhaseUserObject::validParams()
{
  InputParameters params = ADVolumeJunctionBaseUserObject::validParams();

  params.addRequiredCoupledVar("A", "Cross-sectional area of connected flow channels");
  params.addRequiredCoupledVar("rhoA", "rho*A of the connected flow channels");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the connected flow channels");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the connected flow channels");

  params.addRequiredCoupledVar("rhoV", "rho*V of the junction");
  params.addRequiredCoupledVar("rhouV", "rho*u*V of the junction");
  params.addRequiredCoupledVar("rhovV", "rho*v*V of the junction");
  params.addRequiredCoupledVar("rhowV", "rho*w*V of the junction");
  params.addRequiredCoupledVar("rhoEV", "rho*E*V of the junction");

  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object name");

  params.addRequiredParam<Real>("K", "Form loss coefficient [-]");
  params.addRequiredParam<Real>("A_ref", "Reference area [m^2]");

  params.addClassDescription(
      "Computes and caches flux and residual vectors for a 1-phase volume junction");

  params.declareControllable("K");
  return params;
}

ADVolumeJunction1PhaseUserObject::ADVolumeJunction1PhaseUserObject(const InputParameters & params)
  : ADVolumeJunctionBaseUserObject(params),

    _A(adCoupledValue("A")),
    _rhoA(adCoupledValue("rhoA")),
    _rhouA(adCoupledValue("rhouA")),
    _rhoEA(adCoupledValue("rhoEA")),

    _K(getParam<Real>("K")),
    _A_ref(getParam<Real>("A_ref")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
  _flow_variable_names.resize(THM3Eqn::N_EQ);
  _flow_variable_names[THM3Eqn::CONS_VAR_RHOA] = "rhoA";
  _flow_variable_names[THM3Eqn::CONS_VAR_RHOUA] = "rhouA";
  _flow_variable_names[THM3Eqn::CONS_VAR_RHOEA] = "rhoEA";

  _scalar_variable_names.resize(VolumeJunction1Phase::N_EQ);
  _scalar_variable_names[VolumeJunction1Phase::RHOV_INDEX] = "rhoV";
  _scalar_variable_names[VolumeJunction1Phase::RHOUV_INDEX] = "rhouV";
  _scalar_variable_names[VolumeJunction1Phase::RHOVV_INDEX] = "rhovV";
  _scalar_variable_names[VolumeJunction1Phase::RHOWV_INDEX] = "rhowV";
  _scalar_variable_names[VolumeJunction1Phase::RHOEV_INDEX] = "rhoEV";

  _junction_var_values.resize(VolumeJunction1Phase::N_EQ);
  _junction_var_values[VolumeJunction1Phase::RHOV_INDEX] = &coupledJunctionValue("rhoV");
  _junction_var_values[VolumeJunction1Phase::RHOUV_INDEX] = &coupledJunctionValue("rhouV");
  _junction_var_values[VolumeJunction1Phase::RHOVV_INDEX] = &coupledJunctionValue("rhovV");
  _junction_var_values[VolumeJunction1Phase::RHOWV_INDEX] = &coupledJunctionValue("rhowV");
  _junction_var_values[VolumeJunction1Phase::RHOEV_INDEX] = &coupledJunctionValue("rhoEV");

  _numerical_flux_uo.resize(_n_connections);
  for (std::size_t i = 0; i < _n_connections; i++)
    _numerical_flux_uo[i] = &getUserObjectByName<ADNumericalFlux3EqnBase>(_numerical_flux_names[i]);
}

void
ADVolumeJunction1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  const Real din = _normal[c];
  const Point di = _dir[0];
  const Point ni = di * din;
  const Real nJi_dot_di = -din;

  std::vector<ADReal> Ui(THM3Eqn::N_CONS_VAR, 0.);
  Ui[THM3Eqn::CONS_VAR_RHOA] = _rhoA[0];
  Ui[THM3Eqn::CONS_VAR_RHOUA] = _rhouA[0];
  Ui[THM3Eqn::CONS_VAR_RHOEA] = _rhoEA[0];
  Ui[THM3Eqn::CONS_VAR_AREA] = _A[0];

  const auto & rhoV = _cached_junction_var_values[VolumeJunction1Phase::RHOV_INDEX];
  const auto & rhouV = _cached_junction_var_values[VolumeJunction1Phase::RHOUV_INDEX];
  const auto & rhovV = _cached_junction_var_values[VolumeJunction1Phase::RHOVV_INDEX];
  const auto & rhowV = _cached_junction_var_values[VolumeJunction1Phase::RHOWV_INDEX];
  const auto & rhoEV = _cached_junction_var_values[VolumeJunction1Phase::RHOEV_INDEX];

  std::vector<ADReal> UJi(THM3Eqn::N_CONS_VAR, 0.);
  const ADRealVectorValue rhouV_vec(rhouV, rhovV, rhowV);
  UJi[THM3Eqn::CONS_VAR_RHOA] = rhoV / _volume * _A[0];
  UJi[THM3Eqn::CONS_VAR_RHOUA] = rhouV_vec * di / _volume * _A[0];
  UJi[THM3Eqn::CONS_VAR_RHOEA] = rhoEV / _volume * _A[0];
  UJi[THM3Eqn::CONS_VAR_AREA] = _A[0];

  _flux[c] =
      _numerical_flux_uo[c]->getFlux(_current_side, _current_elem->id(), true, UJi, Ui, nJi_dot_di);

  const ADReal vJ = THM::v_from_rhoA_A(rhoV, _volume);
  const ADReal rhouV2 = rhouV_vec * rhouV_vec;
  const ADReal eJ = rhoEV / rhoV - 0.5 * rhouV2 / (rhoV * rhoV);
  const ADReal pJ = _fp.p_from_v_e(vJ, eJ);

  if (c == 0 && std::abs(_K) > 1e-10)
  {
    const ADReal vel_in = _rhouA[0] / _rhoA[0];
    const ADReal v_in = THM::v_from_rhoA_A(_rhoA[0], _A[0]);
    const ADReal rhouA2 = _rhouA[0] * _rhouA[0];
    const ADReal e_in = _rhoEA[0] / _rhoA[0] - 0.5 * rhouA2 / (_rhoA[0] * _rhoA[0]);
    const ADReal p_in = _fp.p_from_v_e(v_in, e_in);
    const ADReal s0_in = _fp.s_from_v_e(v_in, e_in);
    const ADReal T_in = _fp.T_from_v_e(v_in, e_in);
    const ADReal h_in = _fp.h_from_p_T(p_in, T_in);
    const ADReal velin2 = vel_in * vel_in;
    const ADReal h0_in = h_in + 0.5 * velin2;
    const ADReal p0_in = _fp.p_from_h_s(h0_in, s0_in);
    ADReal S_loss;
    if (_A_ref == 0)
      S_loss = _K * (p0_in - p_in) * _A[0];
    else
      S_loss = _K * (p0_in - p_in) * _A_ref;
    if (THM::isInlet(vel_in, _normal[c]))
    {
      _residual[VolumeJunction1Phase::RHOUV_INDEX] -= ni(0) * S_loss;
      _residual[VolumeJunction1Phase::RHOVV_INDEX] -= ni(1) * S_loss;
      _residual[VolumeJunction1Phase::RHOWV_INDEX] -= ni(2) * S_loss;
    }
    else
    {
      _residual[VolumeJunction1Phase::RHOUV_INDEX] += ni(0) * S_loss;
      _residual[VolumeJunction1Phase::RHOVV_INDEX] += ni(1) * S_loss;
      _residual[VolumeJunction1Phase::RHOWV_INDEX] += ni(2) * S_loss;
    }
    _residual[VolumeJunction1Phase::RHOEV_INDEX] += S_loss * std::abs(vel_in);
  }

  _residual[VolumeJunction1Phase::RHOV_INDEX] -= din * _flux[c][THM3Eqn::CONS_VAR_RHOA];
  _residual[VolumeJunction1Phase::RHOUV_INDEX] -=
      di(0) * din * _flux[c][THM3Eqn::CONS_VAR_RHOUA] - pJ * ni(0) * _A[0];
  _residual[VolumeJunction1Phase::RHOVV_INDEX] -=
      di(1) * din * _flux[c][THM3Eqn::CONS_VAR_RHOUA] - pJ * ni(1) * _A[0];
  _residual[VolumeJunction1Phase::RHOWV_INDEX] -=
      di(2) * din * _flux[c][THM3Eqn::CONS_VAR_RHOUA] - pJ * ni(2) * _A[0];
  _residual[VolumeJunction1Phase::RHOEV_INDEX] -= din * _flux[c][THM3Eqn::CONS_VAR_RHOEA];
}

void
ADVolumeJunction1PhaseUserObject::finalize()
{
  ADVolumeJunctionBaseUserObject::finalize();
  for (unsigned int i = 0; i < _n_scalar_eq; i++)
    comm().sum(_residual[i]);
}
