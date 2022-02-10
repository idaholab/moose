//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunction1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndices3Eqn.h"
#include "VolumeJunction1Phase.h"
#include "NumericalFlux3EqnBase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", VolumeJunction1PhaseUserObject);

InputParameters
VolumeJunction1PhaseUserObject::validParams()
{
  InputParameters params = VolumeJunctionBaseUserObject::validParams();

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

  return params;
}

VolumeJunction1PhaseUserObject::VolumeJunction1PhaseUserObject(const InputParameters & params)
  : VolumeJunctionBaseUserObject(params),

    _A(coupledValue("A")),
    _rhoA(coupledValue("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _rhoEA(coupledValue("rhoEA")),

    _rhoV(coupledScalarValue("rhoV")),
    _rhouV(coupledScalarValue("rhouV")),
    _rhovV(coupledScalarValue("rhovV")),
    _rhowV(coupledScalarValue("rhowV")),
    _rhoEV(coupledScalarValue("rhoEV")),

    _rhoA_jvar(coupled("rhoA")),
    _rhouA_jvar(coupled("rhouA")),
    _rhoEA_jvar(coupled("rhoEA")),

    _rhoV_jvar(coupledScalar("rhoV")),
    _rhouV_jvar(coupledScalar("rhouV")),
    _rhovV_jvar(coupledScalar("rhovV")),
    _rhowV_jvar(coupledScalar("rhowV")),
    _rhoEV_jvar(coupledScalar("rhoEV")),

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

  _numerical_flux_uo.resize(_n_connections);
  for (std::size_t i = 0; i < _n_connections; i++)
    _numerical_flux_uo[i] = &getUserObjectByName<NumericalFlux3EqnBase>(_numerical_flux_names[i]);
}

void
VolumeJunction1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  const Real din = _normal[c];
  const Point di = _dir[0];
  const Point ni = di * din;
  const Real nJi_dot_di = -din;

  std::vector<Real> Ui(THM3Eqn::N_CONS_VAR, 0.);
  Ui[THM3Eqn::CONS_VAR_RHOA] = _rhoA[0];
  Ui[THM3Eqn::CONS_VAR_RHOUA] = _rhouA[0];
  Ui[THM3Eqn::CONS_VAR_RHOEA] = _rhoEA[0];
  Ui[THM3Eqn::CONS_VAR_AREA] = _A[0];

  std::vector<Real> UJi(THM3Eqn::N_CONS_VAR, 0.);
  const RealVectorValue rhouV_vec(_rhouV[0], _rhovV[0], _rhowV[0]);
  UJi[THM3Eqn::CONS_VAR_RHOA] = _rhoV[0] / _volume * _A[0];
  UJi[THM3Eqn::CONS_VAR_RHOUA] = rhouV_vec * di / _volume * _A[0];
  UJi[THM3Eqn::CONS_VAR_RHOEA] = _rhoEV[0] / _volume * _A[0];
  UJi[THM3Eqn::CONS_VAR_AREA] = _A[0];

  _flux[c] =
      _numerical_flux_uo[c]->getFlux(_current_side, _current_elem->id(), true, UJi, Ui, nJi_dot_di);

  Real vJ, dvJ_drhoV;
  THM::v_from_rhoA_A(_rhoV[0], _volume, vJ, dvJ_drhoV);

  const Real rhouV2 = rhouV_vec * rhouV_vec;
  const Real eJ = _rhoEV[0] / _rhoV[0] - 0.5 * rhouV2 / (_rhoV[0] * _rhoV[0]);
  const Real deJ_drhoV = -_rhoEV[0] / (_rhoV[0] * _rhoV[0]) + rhouV2 / std::pow(_rhoV[0], 3);
  const Real deJ_drhouV = -_rhouV[0] / (_rhoV[0] * _rhoV[0]);
  const Real deJ_drhovV = -_rhovV[0] / (_rhoV[0] * _rhoV[0]);
  const Real deJ_drhowV = -_rhowV[0] / (_rhoV[0] * _rhoV[0]);
  const Real deJ_drhoEV = 1.0 / _rhoV[0];

  Real pJ, dpJ_dvJ, dpJ_deJ;
  _fp.p_from_v_e(vJ, eJ, pJ, dpJ_dvJ, dpJ_deJ);
  std::vector<Real> dpJ_dUJ(_n_scalar_eq, 0);
  dpJ_dUJ[VolumeJunction1Phase::RHOV_INDEX] = dpJ_dvJ * dvJ_drhoV + dpJ_deJ * deJ_drhoV;
  dpJ_dUJ[VolumeJunction1Phase::RHOUV_INDEX] = dpJ_deJ * deJ_drhouV;
  dpJ_dUJ[VolumeJunction1Phase::RHOVV_INDEX] = dpJ_deJ * deJ_drhovV;
  dpJ_dUJ[VolumeJunction1Phase::RHOWV_INDEX] = dpJ_deJ * deJ_drhowV;
  dpJ_dUJ[VolumeJunction1Phase::RHOEV_INDEX] = dpJ_deJ * deJ_drhoEV;

  std::vector<Real> dS_loss_dUJ(_n_scalar_eq, 0);
  if (c == 0)
  {
    const RealVectorValue velJ = rhouV_vec / _rhoV[0];

    Real s0J, ds0J_dvJ, ds0J_deJ;
    _fp.s_from_v_e(vJ, eJ, s0J, ds0J_dvJ, ds0J_deJ);
    const Real ds0J_drhoV = ds0J_dvJ * dvJ_drhoV + ds0J_deJ * deJ_drhoV;
    const Real ds0J_drhouV = ds0J_deJ * deJ_drhouV;
    const Real ds0J_drhovV = ds0J_deJ * deJ_drhovV;
    const Real ds0J_drhowV = ds0J_deJ * deJ_drhowV;
    const Real ds0J_drhoEV = ds0J_deJ * deJ_drhoEV;

    Real TJ, dTJ_dvJ, dTJ_deJ;
    _fp.T_from_v_e(vJ, eJ, TJ, dTJ_dvJ, dTJ_deJ);
    const Real dTJ_drhoV = dTJ_dvJ * dvJ_drhoV + dTJ_deJ * deJ_drhoV;
    const Real dTJ_drhouV = dTJ_deJ * deJ_drhouV;
    const Real dTJ_drhovV = dTJ_deJ * deJ_drhovV;
    const Real dTJ_drhowV = dTJ_deJ * deJ_drhowV;
    const Real dTJ_drhoEV = dTJ_deJ * deJ_drhoEV;

    Real hJ, dhJ_dpJ, dhJ_dTJ;
    _fp.h_from_p_T(pJ, TJ, hJ, dhJ_dpJ, dhJ_dTJ);
    const Real dhJ_drhoV =
        dhJ_dpJ * dpJ_dUJ[VolumeJunction1Phase::RHOV_INDEX] + dhJ_dTJ * dTJ_drhoV;
    const Real dhJ_drhouV =
        dhJ_dpJ * dpJ_dUJ[VolumeJunction1Phase::RHOUV_INDEX] + dhJ_dTJ * dTJ_drhouV;
    const Real dhJ_drhovV =
        dhJ_dpJ * dpJ_dUJ[VolumeJunction1Phase::RHOVV_INDEX] + dhJ_dTJ * dTJ_drhovV;
    const Real dhJ_drhowV =
        dhJ_dpJ * dpJ_dUJ[VolumeJunction1Phase::RHOWV_INDEX] + dhJ_dTJ * dTJ_drhowV;
    const Real dhJ_drhoEV =
        dhJ_dpJ * dpJ_dUJ[VolumeJunction1Phase::RHOEV_INDEX] + dhJ_dTJ * dTJ_drhoEV;

    const Real velJ2 = velJ * velJ;
    const Real dvelJ2_drhoV = -2. * velJ2 / _rhoV[0];
    const Real dvelJ2_drhouV = 2. * _rhouV[0] / _rhoV[0] / _rhoV[0];
    const Real dvelJ2_drhovV = 2. * _rhovV[0] / _rhoV[0] / _rhoV[0];
    const Real dvelJ2_drhowV = 2. * _rhowV[0] / _rhoV[0] / _rhoV[0];

    const Real h0J = hJ + 0.5 * velJ2;
    const Real dh0J_drhoV = dhJ_drhoV + 0.5 * dvelJ2_drhoV;
    const Real dh0J_drhouV = dhJ_drhouV + 0.5 * dvelJ2_drhouV;
    const Real dh0J_drhovV = dhJ_drhovV + 0.5 * dvelJ2_drhovV;
    const Real dh0J_drhowV = dhJ_drhowV + 0.5 * dvelJ2_drhowV;
    const Real dh0J_drhoEV = dhJ_drhoEV;

    Real p0J, dp0J_dh0J, dp0J_ds0J;
    _fp.p_from_h_s(h0J, s0J, p0J, dp0J_dh0J, dp0J_ds0J);
    const Real dp0J_drhoV = dp0J_dh0J * dh0J_drhoV + dp0J_ds0J * ds0J_drhoV;
    const Real dp0J_drhouV = dp0J_dh0J * dh0J_drhouV + dp0J_ds0J * ds0J_drhouV;
    const Real dp0J_drhovV = dp0J_dh0J * dh0J_drhovV + dp0J_ds0J * ds0J_drhovV;
    const Real dp0J_drhowV = dp0J_dh0J * dh0J_drhowV + dp0J_ds0J * ds0J_drhowV;
    const Real dp0J_drhoEV = dp0J_dh0J * dh0J_drhoEV + dp0J_ds0J * ds0J_drhoEV;

    const Real S_loss = _K * (p0J - pJ) * _A_ref;

    _residual[VolumeJunction1Phase::RHOUV_INDEX] += ni(0) * S_loss;
    _residual[VolumeJunction1Phase::RHOVV_INDEX] += ni(1) * S_loss;
    _residual[VolumeJunction1Phase::RHOWV_INDEX] += ni(2) * S_loss;

    dS_loss_dUJ[VolumeJunction1Phase::RHOV_INDEX] =
        _K * (dp0J_drhoV - dpJ_dUJ[VolumeJunction1Phase::RHOV_INDEX]) * _A_ref;
    dS_loss_dUJ[VolumeJunction1Phase::RHOUV_INDEX] =
        _K * (dp0J_drhouV - dpJ_dUJ[VolumeJunction1Phase::RHOUV_INDEX]) * _A_ref;
    dS_loss_dUJ[VolumeJunction1Phase::RHOVV_INDEX] =
        _K * (dp0J_drhovV - dpJ_dUJ[VolumeJunction1Phase::RHOVV_INDEX]) * _A_ref;
    dS_loss_dUJ[VolumeJunction1Phase::RHOWV_INDEX] =
        _K * (dp0J_drhowV - dpJ_dUJ[VolumeJunction1Phase::RHOWV_INDEX]) * _A_ref;
    dS_loss_dUJ[VolumeJunction1Phase::RHOEV_INDEX] =
        _K * (dp0J_drhoEV - dpJ_dUJ[VolumeJunction1Phase::RHOEV_INDEX]) * _A_ref;
  }

  _residual[VolumeJunction1Phase::RHOV_INDEX] -= din * _flux[c][THM3Eqn::CONS_VAR_RHOA];
  _residual[VolumeJunction1Phase::RHOUV_INDEX] -=
      di(0) * din * _flux[c][THM3Eqn::CONS_VAR_RHOUA] - pJ * ni(0) * _A[0];
  _residual[VolumeJunction1Phase::RHOVV_INDEX] -=
      di(1) * din * _flux[c][THM3Eqn::CONS_VAR_RHOUA] - pJ * ni(1) * _A[0];
  _residual[VolumeJunction1Phase::RHOWV_INDEX] -=
      di(2) * din * _flux[c][THM3Eqn::CONS_VAR_RHOUA] - pJ * ni(2) * _A[0];
  _residual[VolumeJunction1Phase::RHOEV_INDEX] -= din * _flux[c][THM3Eqn::CONS_VAR_RHOEA];

  // Compute flux Jacobian w.r.t. scalar variables
  const DenseMatrix<Real> dflux_dUJi = _numerical_flux_uo[c]->getJacobian(
      true, true, _current_side, _current_elem->id(), UJi, Ui, nJi_dot_di);
  for (unsigned int i = 0; i < _n_flux_eq; i++)
  {
    _flux_jacobian_scalar_vars[c](i, VolumeJunction1Phase::RHOV_INDEX) =
        dflux_dUJi(i, THM3Eqn::CONS_VAR_RHOA) * _A[0] / _volume;
    _flux_jacobian_scalar_vars[c](i, VolumeJunction1Phase::RHOUV_INDEX) =
        dflux_dUJi(i, THM3Eqn::CONS_VAR_RHOUA) * di(0) * _A[0] / _volume;
    _flux_jacobian_scalar_vars[c](i, VolumeJunction1Phase::RHOVV_INDEX) =
        dflux_dUJi(i, THM3Eqn::CONS_VAR_RHOUA) * di(1) * _A[0] / _volume;
    _flux_jacobian_scalar_vars[c](i, VolumeJunction1Phase::RHOWV_INDEX) =
        dflux_dUJi(i, THM3Eqn::CONS_VAR_RHOUA) * di(2) * _A[0] / _volume;
    _flux_jacobian_scalar_vars[c](i, VolumeJunction1Phase::RHOEV_INDEX) =
        dflux_dUJi(i, THM3Eqn::CONS_VAR_RHOEA) * _A[0] / _volume;
  }

  // Compute flux Jacobian w.r.t. flow channel variables
  _flux_jacobian_flow_channel_vars[c] = _numerical_flux_uo[c]->getJacobian(
      true, false, _current_side, _current_elem->id(), UJi, Ui, nJi_dot_di);

  for (unsigned int i = 0; i < _n_scalar_eq; i++)
  {
    // Cache scalar residual Jacobian entries w.r.t. scalar variables
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOV_INDEX](0, i) -=
        din * _flux_jacobian_scalar_vars[c](THM3Eqn::CONS_VAR_RHOA, i);
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOUV_INDEX](0, i) -=
        di(0) * din * _flux_jacobian_scalar_vars[c](THM3Eqn::CONS_VAR_RHOUA, i) -
        dpJ_dUJ[i] * ni(0) * _A[0];
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOVV_INDEX](0, i) -=
        di(1) * din * _flux_jacobian_scalar_vars[c](THM3Eqn::CONS_VAR_RHOUA, i) -
        dpJ_dUJ[i] * ni(1) * _A[0];
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOWV_INDEX](0, i) -=
        di(2) * din * _flux_jacobian_scalar_vars[c](THM3Eqn::CONS_VAR_RHOUA, i) -
        dpJ_dUJ[i] * ni(2) * _A[0];
    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOEV_INDEX](0, i) -=
        din * _flux_jacobian_scalar_vars[c](THM3Eqn::CONS_VAR_RHOEA, i);
  }
  if (c == 0)
  {
    for (unsigned int i = 0; i < _n_scalar_eq; i++)
    {
      _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOUV_INDEX](0, i) +=
          ni(0) * dS_loss_dUJ[i];
      _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOVV_INDEX](0, i) +=
          ni(1) * dS_loss_dUJ[i];
      _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOWV_INDEX](0, i) +=
          ni(2) * dS_loss_dUJ[i];
    }
  }

  // Compute Jacobian w.r.t. flow channel solution function
  DenseMatrix<Real> jac(_n_scalar_eq, _n_flux_eq);
  for (unsigned int j = 0; j < _n_flux_eq; j++)
  {
    jac(VolumeJunction1Phase::RHOV_INDEX, j) =
        -din * _flux_jacobian_flow_channel_vars[c](THM3Eqn::CONS_VAR_RHOA, j);
    jac(VolumeJunction1Phase::RHOUV_INDEX, j) =
        -di(0) * din * _flux_jacobian_flow_channel_vars[c](THM3Eqn::CONS_VAR_RHOUA, j);
    jac(VolumeJunction1Phase::RHOVV_INDEX, j) =
        -di(1) * din * _flux_jacobian_flow_channel_vars[c](THM3Eqn::CONS_VAR_RHOUA, j);
    jac(VolumeJunction1Phase::RHOWV_INDEX, j) =
        -di(2) * din * _flux_jacobian_flow_channel_vars[c](THM3Eqn::CONS_VAR_RHOUA, j);
    jac(VolumeJunction1Phase::RHOEV_INDEX, j) =
        -din * _flux_jacobian_flow_channel_vars[c](THM3Eqn::CONS_VAR_RHOEA, j);
  }
  computeScalarJacobianWRTFlowDofs(jac, c);
}
