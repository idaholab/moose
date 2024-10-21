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
#include "THMIndicesVACE.h"
#include "VolumeJunction1Phase.h"
#include "ADNumericalFlux3EqnBase.h"
#include "Numerics.h"
#include "THMUtils.h"
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

  params.addParam<bool>("apply_velocity_scaling",
                        false,
                        "Set to true to apply the scaling to the normal velocity. See "
                        "documentation for more information.");

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

    _apply_velocity_scaling(getParam<bool>("apply_velocity_scaling")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
  _flow_variable_names.resize(THMVACE1D::N_FLUX_OUTPUTS);
  _flow_variable_names[THMVACE1D::RHOA] = "rhoA";
  _flow_variable_names[THMVACE1D::RHOUA] = "rhouA";
  _flow_variable_names[THMVACE1D::RHOEA] = "rhoEA";

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
  const RealVectorValue di = _dir[0];
  const RealVectorValue ni = di * din;
  const RealVectorValue nJi = -ni;
  const Real nJi_dot_di = -din;

  RealVectorValue t1, t2;
  THM::computeOrthogonalDirections(nJi, t1, t2);

  std::vector<ADReal> Ui(THMVACE3D::N_FLUX_INPUTS, 0.);
  Ui[THMVACE3D::RHOA] = _rhoA[0];
  Ui[THMVACE3D::RHOUA] = _rhouA[0] * di(0);
  Ui[THMVACE3D::RHOVA] = _rhouA[0] * di(1);
  Ui[THMVACE3D::RHOWA] = _rhouA[0] * di(2);
  Ui[THMVACE3D::RHOEA] = _rhoEA[0];
  Ui[THMVACE3D::AREA] = _A[0];

  const auto & rhoV = _cached_junction_var_values[VolumeJunction1Phase::RHOV_INDEX];
  const auto & rhouV = _cached_junction_var_values[VolumeJunction1Phase::RHOUV_INDEX];
  const auto & rhovV = _cached_junction_var_values[VolumeJunction1Phase::RHOVV_INDEX];
  const auto & rhowV = _cached_junction_var_values[VolumeJunction1Phase::RHOWV_INDEX];
  const auto & rhoEV = _cached_junction_var_values[VolumeJunction1Phase::RHOEV_INDEX];

  const ADReal rhoJ = rhoV / _volume;
  const ADReal vJ = 1.0 / rhoJ;
  const ADRealVectorValue uvecJ(rhouV / rhoV, rhovV / rhoV, rhowV / rhoV);
  const ADReal eJ = rhoEV / rhoV - 0.5 * uvecJ * uvecJ;
  const ADReal pJ = _fp.p_from_v_e(vJ, eJ);

  std::vector<ADReal> UJi(THMVACE3D::N_FLUX_INPUTS, 0.);
  UJi[THMVACE3D::RHOA] = rhoV / _volume * _A[0];
  if (_apply_velocity_scaling)
  {
    const ADReal unJ = uvecJ * nJi;
    const ADReal ut1J = uvecJ * t1;
    const ADReal ut2J = uvecJ * t2;
    const ADReal uni = _rhouA[0] / _rhoA[0] * nJi_dot_di;

    const ADReal rhoi = _rhoA[0] / _A[0];
    const ADReal vi = 1.0 / rhoi;
    const ADReal ei = _rhoEA[0] / _rhoA[0] - 0.5 * uni * uni;
    const ADReal ci = _fp.c_from_v_e(vi, ei);
    const ADReal cJ = _fp.c_from_v_e(vJ, eJ);
    const ADReal cmax = std::max(ci, cJ);

    const ADReal uni_sign = (uni > 0) - (uni < 0);
    const ADReal factor = 0.5 * (1.0 - uni_sign) * std::min(std::abs(uni - unJ) / cmax, 1.0);

    const ADReal unJ_mod = uni - factor * (uni - unJ);
    const ADRealVectorValue uvecJ_mod = unJ_mod * nJi + ut1J * t1 + ut2J * t2;
    const ADReal EJ_mod = eJ + 0.5 * uvecJ_mod * uvecJ_mod;

    UJi[THMVACE3D::RHOUA] = rhoJ * uvecJ_mod(0) * _A[0];
    UJi[THMVACE3D::RHOVA] = rhoJ * uvecJ_mod(1) * _A[0];
    UJi[THMVACE3D::RHOWA] = rhoJ * uvecJ_mod(2) * _A[0];
    UJi[THMVACE3D::RHOEA] = rhoJ * EJ_mod * _A[0];
  }
  else
  {
    UJi[THMVACE3D::RHOUA] = rhouV / _volume * _A[0];
    UJi[THMVACE3D::RHOVA] = rhovV / _volume * _A[0];
    UJi[THMVACE3D::RHOWA] = rhowV / _volume * _A[0];
    UJi[THMVACE3D::RHOEA] = rhoEV / _volume * _A[0];
  }
  UJi[THMVACE3D::AREA] = _A[0];

  const auto flux_3d = _numerical_flux_uo[c]->getFlux3D(
      _current_side, _current_elem->id(), true, UJi, Ui, nJi, t1, t2);

  _flux[c].resize(THMVACE1D::N_FLUX_OUTPUTS);
  _flux[c][THMVACE1D::MASS] = flux_3d[THMVACE3D::MASS] * nJi_dot_di;
  _flux[c][THMVACE1D::MOMENTUM] = flux_3d[THMVACE3D::MOM_NORM];
  _flux[c][THMVACE1D::ENERGY] = flux_3d[THMVACE3D::ENERGY] * nJi_dot_di;

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

  const ADRealVectorValue flux_mom_n = flux_3d[THMVACE3D::MOM_NORM] * nJi +
                                       flux_3d[THMVACE3D::MOM_TAN1] * t1 +
                                       flux_3d[THMVACE3D::MOM_TAN2] * t2;
  const RealVectorValue ex(1, 0, 0);
  const RealVectorValue ey(0, 1, 0);
  const RealVectorValue ez(0, 0, 1);

  _residual[VolumeJunction1Phase::RHOV_INDEX] -= -flux_3d[THMVACE3D::RHOA];
  _residual[VolumeJunction1Phase::RHOUV_INDEX] -= -flux_mom_n * ex - pJ * ni(0) * _A[0];
  _residual[VolumeJunction1Phase::RHOVV_INDEX] -= -flux_mom_n * ey - pJ * ni(1) * _A[0];
  _residual[VolumeJunction1Phase::RHOWV_INDEX] -= -flux_mom_n * ez - pJ * ni(2) * _A[0];
  _residual[VolumeJunction1Phase::RHOEV_INDEX] -= -flux_3d[THMVACE3D::RHOEA];
}

void
ADVolumeJunction1PhaseUserObject::finalize()
{
  ADVolumeJunctionBaseUserObject::finalize();
  for (unsigned int i = 0; i < _n_scalar_eq; i++)
    comm().sum(_residual[i]);
}
