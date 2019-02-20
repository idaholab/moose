#include "VolumeJunction1PhaseUserObject.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndices3Eqn.h"
#include "VolumeJunction1Phase.h"
#include "RDGFluxBase.h"
#include "Numerics.h"

registerMooseObject("THMApp", VolumeJunction1PhaseUserObject);

template <>
InputParameters
validParams<VolumeJunction1PhaseUserObject>()
{
  InputParameters params = validParams<VolumeJunctionBaseUserObject>();

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

  params.addClassDescription(
      "Computes and caches flux and residual vectors for a 1-phase volume junction");

  return params;
}

VolumeJunction1PhaseUserObject::VolumeJunction1PhaseUserObject(const InputParameters & params)
  : VolumeJunctionBaseUserObject(
        params, getCoupledFlowVariableNames(), getCoupledScalarVariableNames()),

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

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
  _numerical_flux_uo.resize(_n_connections);
  for (std::size_t i = 0; i < _n_connections; i++)
    _numerical_flux_uo[i] = &getUserObjectByName<RDGFluxBase>(_numerical_flux_names[i]);
}

void
VolumeJunction1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  const Real din = _normal[c];
  const Point di = _dir[0];
  const Point ni = di * din;
  const Point nJi = -ni;

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

  _flux[c] = _numerical_flux_uo[c]->getFlux(_current_side, _current_elem->id(), UJi, Ui, nJi);

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

  _residual[VolumeJunction1Phase::RHOV_INDEX] -= din * _flux[c][THM3Eqn::CONS_VAR_RHOA];
  _residual[VolumeJunction1Phase::RHOUV_INDEX] -=
      di(0) * din * _flux[c][THM3Eqn::CONS_VAR_RHOUA] - pJ * ni(0) * _A[0];
  _residual[VolumeJunction1Phase::RHOVV_INDEX] -=
      di(1) * din * _flux[c][THM3Eqn::CONS_VAR_RHOUA] - pJ * ni(1) * _A[0];
  _residual[VolumeJunction1Phase::RHOWV_INDEX] -=
      di(2) * din * _flux[c][THM3Eqn::CONS_VAR_RHOUA] - pJ * ni(2) * _A[0];
  _residual[VolumeJunction1Phase::RHOEV_INDEX] -= din * _flux[c][THM3Eqn::CONS_VAR_RHOEA];

  // Compute flux Jacobian w.r.t. scalar variables
  const DenseMatrix<Real> dflux_dUJi =
      _numerical_flux_uo[c]->getJacobian(true, _current_side, _current_elem->id(), UJi, Ui, nJi);
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
  _flux_jacobian_flow_channel_vars[c] =
      _numerical_flux_uo[c]->getJacobian(false, _current_side, _current_elem->id(), UJi, Ui, nJi);

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

std::vector<std::string>
VolumeJunction1PhaseUserObject::getCoupledFlowVariableNames()
{
  std::vector<std::string> flow_variable_names(THM3Eqn::N_EQ);
  flow_variable_names[THM3Eqn::CONS_VAR_RHOA] = "rhoA";
  flow_variable_names[THM3Eqn::CONS_VAR_RHOUA] = "rhouA";
  flow_variable_names[THM3Eqn::CONS_VAR_RHOEA] = "rhoEA";

  return flow_variable_names;
}

std::vector<std::string>
VolumeJunction1PhaseUserObject::getCoupledScalarVariableNames()
{
  std::vector<std::string> scalar_variable_names(VolumeJunction1Phase::N_EQ);
  scalar_variable_names[VolumeJunction1Phase::RHOV_INDEX] = "rhoV";
  scalar_variable_names[VolumeJunction1Phase::RHOUV_INDEX] = "rhouV";
  scalar_variable_names[VolumeJunction1Phase::RHOVV_INDEX] = "rhovV";
  scalar_variable_names[VolumeJunction1Phase::RHOWV_INDEX] = "rhowV";
  scalar_variable_names[VolumeJunction1Phase::RHOEV_INDEX] = "rhoEV";

  return scalar_variable_names;
}
