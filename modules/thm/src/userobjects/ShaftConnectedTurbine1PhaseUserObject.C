#include "ShaftConnectedTurbine1PhaseUserObject.h"
#include "ShaftConnectedTurbine1Phase.h"
#include "SinglePhaseFluidProperties.h"
#include "VolumeJunction1Phase.h"
#include "MooseVariableScalar.h"
#include "THMIndices3Eqn.h"
#include "Function.h"
#include "Numerics.h"

registerMooseObject("THMApp", ShaftConnectedTurbine1PhaseUserObject);

InputParameters
ShaftConnectedTurbine1PhaseUserObject::validParams()
{
  InputParameters params = VolumeJunction1PhaseUserObject::validParams();
  params += ShaftConnectableUserObjectInterface::validParams();

  params.addParam<BoundaryName>("inlet", "Turbine inlet");
  params.addParam<BoundaryName>("outlet", "Turbine outlet");
  params.addRequiredParam<Point>("di_out", "Direction of connected outlet");
  params.addRequiredParam<Real>("omega_rated", "Rated turbine speed [rad/s]");
  params.addRequiredParam<Real>("D_wheel", "Diameter of turbine wheel [m]");
  params.addRequiredParam<Real>("speed_cr_fr", "Turbine speed threshold for friction [-]");
  params.addRequiredParam<Real>("tau_fr_const", "Turbine friction constant [N-m]");
  params.addRequiredParam<std::vector<Real>>("tau_fr_coeff", "Friction coefficients [N-m]");
  params.addRequiredParam<Real>("speed_cr_I", "Turbine speed threshold for inertia [-]");
  params.addRequiredParam<Real>("inertia_const", "Turbine inertia constant [kg-m^2]");
  params.addRequiredParam<std::vector<Real>>("inertia_coeff",
                                             "Turbine inertia coefficients [kg-m^2]");
  params.addRequiredParam<FunctionName>("head_coefficient",
                                        "Function to compute data for turbine head [-]");
  params.addRequiredParam<FunctionName>("power_coefficient",
                                        "Function to compute data for turbine power [-]");
  params.addRequiredParam<std::string>("turbine_name",
                                       "Name of the instance of this turbine component");
  params.addRequiredCoupledVar("omega", "Shaft rotational speed [rad/s]");

  params.addClassDescription("Computes and caches flux and residual vectors for a 1-phase "
                             "turbine. Also computes turbine torque "
                             "and delta_p which is passed to the connected shaft");

  return params;
}

ShaftConnectedTurbine1PhaseUserObject::ShaftConnectedTurbine1PhaseUserObject(
    const InputParameters & params)
  : VolumeJunction1PhaseUserObject(params),
    ShaftConnectableUserObjectInterface(this),

    _di_out(getParam<Point>("di_out")),
    _omega_rated(getParam<Real>("omega_rated")),
    _D_wheel(getParam<Real>("D_wheel")),
    _speed_cr_fr(getParam<Real>("speed_cr_fr")),
    _tau_fr_const(getParam<Real>("tau_fr_const")),
    _tau_fr_coeff(getParam<std::vector<Real>>("tau_fr_coeff")),
    _speed_cr_I(getParam<Real>("speed_cr_I")),
    _inertia_const(getParam<Real>("inertia_const")),
    _inertia_coeff(getParam<std::vector<Real>>("inertia_coeff")),
    _head_coefficient(getFunction("head_coefficient")),
    _power_coefficient(getFunction("power_coefficient")),
    _turbine_name(getParam<std::string>("turbine_name")),

    _omega(coupledScalarValue("omega"))
{
  unsigned int n_jct_vars = _scalar_variable_names.size();
  _torque_jacobian_scalar_vars.resize(_n_shaft_eq, n_jct_vars);
  _moi_jacobian_scalar_vars.resize(_n_shaft_eq, n_jct_vars);
  _moi_jacobian_scalar_vars.zero();
}

void
ShaftConnectedTurbine1PhaseUserObject::initialSetup()
{
  VolumeJunctionBaseUserObject::initialSetup();

  ShaftConnectableUserObjectInterface::setupConnections(
      VolumeJunctionBaseUserObject::_n_connections, VolumeJunctionBaseUserObject::_n_flux_eq);

  _residual_jacobian_omega_var.resize(_n_scalar_eq);
  for (auto && v : _residual_jacobian_omega_var)
    v.resize(1, ShaftConnectableUserObjectInterface::_n_shaft_eq);
}

void
ShaftConnectedTurbine1PhaseUserObject::initialize()
{
  VolumeJunctionBaseUserObject::initialize();
  ShaftConnectableUserObjectInterface::initialize();
  for (unsigned int i = 0; i < _n_scalar_eq; i++)
    _residual_jacobian_omega_var[i].zero();

  _driving_torque = 0;
  _friction_torque = 0;
  _flow_coeff = 0;
  _delta_p = 0;
  _power = 0;
}

void
ShaftConnectedTurbine1PhaseUserObject::execute()
{
  VolumeJunctionBaseUserObject::storeConnectionData();
  ShaftConnectableUserObjectInterface::setConnectionData(
      VolumeJunctionBaseUserObject::_phi_face_values,
      VolumeJunctionBaseUserObject::_flow_channel_dofs);

  const unsigned int c = getBoundaryIDIndex();

  computeFluxesAndResiduals(c);
}

void
ShaftConnectedTurbine1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  VolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  // inlet c=0 established in component
  if (c == 0)
  {
    const Real Q_in = (_rhouA[0] / _rhoA[0]) * _A[0];
    const Real dQ_in_drhoA = -_rhouA[0] * _A[0] / (_rhoA[0] * _rhoA[0]);
    const Real dQ_in_drhouA = _A[0] / _rhoA[0];

    _flow_coeff = Q_in / (_omega[0] * _D_wheel * _D_wheel * _D_wheel);
    const Real dflow_drhoA = dQ_in_drhoA / (_omega[0] * _D_wheel * _D_wheel * _D_wheel);
    const Real dflow_drhouA = dQ_in_drhouA / (_omega[0] * _D_wheel * _D_wheel * _D_wheel);
    const Real dflow_domega = -Q_in / (_omega[0] * _omega[0] * _D_wheel * _D_wheel * _D_wheel);

    const Real head_coeff = _head_coefficient.value(_flow_coeff, Point());
    const Real dhead_dflow = _head_coefficient.timeDerivative(_flow_coeff, Point());
    const Real dhead_drhoA = dhead_dflow * dflow_drhoA;
    const Real dhead_drhouA = dhead_dflow * dflow_drhouA;
    const Real dhead_domega = dhead_dflow * dflow_domega;

    const Real gH = head_coeff * _D_wheel * _D_wheel * _omega[0] * _omega[0];
    const Real dgH_drhoA = dhead_drhoA * _D_wheel * _D_wheel * _omega[0] * _omega[0];
    const Real dgH_drhouA = dhead_drhouA * _D_wheel * _D_wheel * _omega[0] * _omega[0];
    const Real dgH_domega = dhead_domega * _D_wheel * _D_wheel * _omega[0] * _omega[0] +
                            2. * head_coeff * _D_wheel * _D_wheel * _omega[0];

    const Real power_coeff = _power_coefficient.value(_flow_coeff, Point());
    const Real dpower_dflow = _power_coefficient.timeDerivative(_flow_coeff, Point());
    const Real dpower_drhoA = dpower_dflow * dflow_drhoA;
    const Real dpower_drhouA = dpower_dflow * dflow_drhouA;
    const Real dpower_domega = dpower_dflow * dflow_domega;

    // friction torque
    Real alpha = _omega[0] / _omega_rated;
    Real dalpha_domega = 1 / _omega_rated;
    Real sign;
    if (_omega[0] >= 0)
      sign = -1;
    else
      sign = 1;
    Real dtau_fr_domega;
    if (alpha < _speed_cr_fr)
    {
      _friction_torque = sign * _tau_fr_const;
      dtau_fr_domega = 0;
    }
    else
    {
      _friction_torque = sign * (_tau_fr_coeff[0] + _tau_fr_coeff[1] * std::abs(alpha) +
                                 _tau_fr_coeff[2] * alpha * alpha +
                                 _tau_fr_coeff[3] * std::abs(alpha * alpha * alpha));
      dtau_fr_domega =
          sign * (_tau_fr_coeff[1] * dalpha_domega + 2 * _tau_fr_coeff[2] * dalpha_domega * alpha +
                  3 * _tau_fr_coeff[3] * alpha * alpha * dalpha_domega);
    }

    _driving_torque =
        power_coeff * (_rhoV[0] / _volume) * _omega[0] * _omega[0] * std::pow(_D_wheel, 5);

    const Real dtau_dr_domega =
        dpower_domega * (_rhoV[0] / _volume) * _omega[0] * _omega[0] * std::pow(_D_wheel, 5) +
        2 * power_coeff * (_rhoV[0] / _volume) * _omega[0] * std::pow(_D_wheel, 5);

    _torque += _driving_torque + _friction_torque;

    const Real dtorque_drhoV =
        power_coeff * (1. / _volume) * _omega[0] * _omega[0] * std::pow(_D_wheel, 5);
    const Real dtorque_drhoA =
        dpower_drhoA * (_rhoV[0] / _volume) * _omega[0] * _omega[0] * std::pow(_D_wheel, 5);
    const Real dtorque_drhouA =
        dpower_drhouA * (_rhoV[0] / _volume) * _omega[0] * _omega[0] * std::pow(_D_wheel, 5);
    const Real dtorque_domega = dtau_dr_domega + dtau_fr_domega;

    Real dmoi_domega;
    if (alpha < _speed_cr_I)
    {
      _moment_of_inertia += _inertia_const;
      dmoi_domega = 0;
    }
    else
    {
      _moment_of_inertia += _inertia_coeff[0] + _inertia_coeff[1] * std::abs(alpha) +
                            _inertia_coeff[2] * alpha * alpha +
                            _inertia_coeff[3] * std::abs(alpha * alpha * alpha);
      dmoi_domega = _inertia_coeff[1] * dalpha_domega +
                    2 * _inertia_coeff[2] * dalpha_domega * alpha +
                    3 * _inertia_coeff[3] * alpha * alpha * dalpha_domega;
    }

    // compute momentum and energy source terms
    // a positive torque value results in a negative S_energy
    _power = _torque * _omega[0];
    const Real S_energy = -_power;

    const Real dS_energy_drhoV = -dtorque_drhoV * _omega[0];
    const Real dS_energy_drhoA = -dtorque_drhoA * _omega[0];
    const Real dS_energy_drhouA = -dtorque_drhouA * _omega[0];
    const Real dS_energy_domega = -(dtorque_domega * _omega[0] + _torque);

    _delta_p = (_rhoV[0] / _volume) * gH;
    const Real ddelta_p_drhoV = (1. / _volume) * gH;
    const Real ddelta_p_drhoA = (_rhoV[0] / _volume) * dgH_drhoA;
    const Real ddelta_p_drhouA = (_rhoV[0] / _volume) * dgH_drhouA;
    const Real ddelta_p_domega = (_rhoV[0] / _volume) * dgH_domega;

    const RealVectorValue S_momentum = -_delta_p * _A_ref * _di_out;
    const RealVectorValue dS_momentum_drhoV = -ddelta_p_drhoV * _A_ref * _di_out;
    const RealVectorValue dS_momentum_drhoA = -ddelta_p_drhoA * _A_ref * _di_out;
    const RealVectorValue dS_momentum_drhouA = -ddelta_p_drhouA * _A_ref * _di_out;
    const RealVectorValue dS_momentum_domega = -ddelta_p_domega * _A_ref * _di_out;

    _residual[VolumeJunction1Phase::RHOEV_INDEX] -= S_energy;

    _residual[VolumeJunction1Phase::RHOUV_INDEX] -= S_momentum(0);
    _residual[VolumeJunction1Phase::RHOVV_INDEX] -= S_momentum(1);
    _residual[VolumeJunction1Phase::RHOWV_INDEX] -= S_momentum(2);

    _residual_jacobian_scalar_vars[VolumeJunction1Phase::RHOEV_INDEX](
        0, VolumeJunction1Phase::RHOV_INDEX) -= dS_energy_drhoV;
    _residual_jacobian_omega_var[VolumeJunction1Phase::RHOEV_INDEX](0, 0) -= dS_energy_domega;

    const RealVectorValue rhovelV_index(VolumeJunction1Phase::RHOUV_INDEX,
                                        VolumeJunction1Phase::RHOVV_INDEX,
                                        VolumeJunction1Phase::RHOWV_INDEX);
    for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    {
      _residual_jacobian_scalar_vars[rhovelV_index(i)](0, VolumeJunction1Phase::RHOV_INDEX) -=
          dS_momentum_drhoV(i);
      _residual_jacobian_omega_var[rhovelV_index(i)](0, 0) -= dS_momentum_domega(i);
    }

    {
      DenseMatrix<Real> jac(_n_scalar_eq, _n_flux_eq);
      jac(VolumeJunction1Phase::RHOEV_INDEX, 0) = -dS_energy_drhoA;
      jac(VolumeJunction1Phase::RHOEV_INDEX, 1) = -dS_energy_drhouA;
      jac(VolumeJunction1Phase::RHOUV_INDEX, 0) = -dS_momentum_drhoA(0);
      jac(VolumeJunction1Phase::RHOUV_INDEX, 1) = -dS_momentum_drhouA(0);
      jac(VolumeJunction1Phase::RHOVV_INDEX, 0) = -dS_momentum_drhoA(1);
      jac(VolumeJunction1Phase::RHOVV_INDEX, 1) = -dS_momentum_drhouA(1);
      jac(VolumeJunction1Phase::RHOWV_INDEX, 0) = -dS_momentum_drhoA(2);
      jac(VolumeJunction1Phase::RHOWV_INDEX, 1) = -dS_momentum_drhouA(2);
      for (unsigned int i = 0; i < _n_scalar_eq; i++)
      {
        unsigned int jk = 0;
        for (unsigned int j = 0; j < _n_flux_eq; j++)
        {
          for (unsigned int k = 0; k < VolumeJunctionBaseUserObject::_phi_face_values[c][j].size();
               k++)
          {
            _residual_jacobian_flow_channel_vars[c][i](0, jk) +=
                jac(i, j) * VolumeJunctionBaseUserObject::_phi_face_values[c][j][k];
            jk++;
          }
        }
      }
    }

    _torque_jacobian_scalar_vars.zero();
    _torque_jacobian_scalar_vars(0, VolumeJunction1Phase::RHOV_INDEX) = dtorque_drhoV;

    _torque_jacobian_omega_var(0, 0) = dtorque_domega;
    _moi_jacobian_omega_var(0, 0) = dmoi_domega;

    // dtau_dUi (i.e. wrt flow variables)
    {
      DenseMatrix<Real> jac(1, THM3Eqn::N_EQ);
      jac.zero();
      jac(0, THM3Eqn::EQ_MASS) = dtorque_drhoA;
      jac(0, THM3Eqn::EQ_MOMENTUM) = dtorque_drhouA;
      computeTorqueScalarJacobianWRTFlowDofs(jac, c);
    }
  }
}

Real
ShaftConnectedTurbine1PhaseUserObject::getDrivingTorque() const
{
  return _driving_torque;
}

Real
ShaftConnectedTurbine1PhaseUserObject::getFrictionTorque() const
{
  return _friction_torque;
}

Real
ShaftConnectedTurbine1PhaseUserObject::getFlowCoefficient() const
{
  return _flow_coeff;
}

Real
ShaftConnectedTurbine1PhaseUserObject::getTurbineDeltaP() const
{
  return _delta_p;
}
Real
ShaftConnectedTurbine1PhaseUserObject::getTurbinePower() const
{
  return _power;
}

void
ShaftConnectedTurbine1PhaseUserObject::finalize()
{
  VolumeJunctionBaseUserObject::finalize();

  ShaftConnectableUserObjectInterface::setupJunctionData(
      VolumeJunctionBaseUserObject::_scalar_dofs);
  ShaftConnectableUserObjectInterface::setOmegaDofs(getScalarVar("omega", 0));
}

void
ShaftConnectedTurbine1PhaseUserObject::threadJoin(const UserObject & uo)
{
  VolumeJunctionBaseUserObject::threadJoin(uo);
  ShaftConnectableUserObjectInterface::threadJoin(uo);

  const ShaftConnectedTurbine1PhaseUserObject & scpuo =
      dynamic_cast<const ShaftConnectedTurbine1PhaseUserObject &>(uo);
  _driving_torque += scpuo._driving_torque;
  _friction_torque += scpuo._friction_torque;
  _flow_coeff += scpuo._flow_coeff;
  _delta_p += scpuo._delta_p;
  _power += scpuo._power;

  _moi_jacobian_omega_var(0, 0) += scpuo._moi_jacobian_omega_var(0, 0);

  _torque_jacobian_omega_var(0, 0) += scpuo._torque_jacobian_omega_var(0, 0);
  _torque_jacobian_scalar_vars(0, VolumeJunction1Phase::RHOV_INDEX) +=
      scpuo._torque_jacobian_scalar_vars(0, VolumeJunction1Phase::RHOV_INDEX);

  for (unsigned int i = 0; i < _n_scalar_eq; i++)
  {
    for (unsigned int j = 0; j < _omega_dof.size(); j++)
    {
      _residual_jacobian_omega_var[i](0, j) += scpuo._residual_jacobian_omega_var[i](0, j);
    }
  }
}

void
ShaftConnectedTurbine1PhaseUserObject::getScalarEquationJacobianData(
    const unsigned int & equation_index,
    DenseMatrix<Real> & jacobian_block,
    std::vector<dof_id_type> & dofs_i,
    std::vector<dof_id_type> & dofs_j) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  dofs_i = {VolumeJunctionBaseUserObject::_scalar_dofs[equation_index]};

  // number of DoFs for the dof_j array
  unsigned int n_dofs = VolumeJunctionBaseUserObject::_scalar_dofs.size() + _omega_dof.size();
  for (unsigned int c = 0; c < VolumeJunctionBaseUserObject::_n_connections; c++)
    n_dofs += _residual_jacobian_flow_channel_vars[c][equation_index].n();

  jacobian_block.resize(1, n_dofs);
  dofs_j.resize(n_dofs);

  unsigned int k = 0;
  // Store Jacobian entries w.r.t. scalar variables
  for (unsigned int j = 0; j < VolumeJunctionBaseUserObject::_scalar_dofs.size(); j++, k++)
  {
    jacobian_block(0, k) = _residual_jacobian_scalar_vars[equation_index](0, j);
    dofs_j[k] = VolumeJunctionBaseUserObject::_scalar_dofs[j];
  }
  // Store Jacobian entries w.r.t. shaft variables
  for (unsigned int j = 0; j < _omega_dof.size(); j++, k++)
  {
    jacobian_block(0, k) = _residual_jacobian_omega_var[equation_index](0, j);
    dofs_j[k] = _omega_dof[j];
  }
  // Store Jacobian entries w.r.t. flow variables
  for (unsigned int c = 0; c < VolumeJunctionBaseUserObject::_n_connections; c++)
  {
    for (unsigned int j = 0; j < _residual_jacobian_flow_channel_vars[c][equation_index].n();
         j++, k++)
    {
      jacobian_block(0, k) = _residual_jacobian_flow_channel_vars[c][equation_index](0, j);
      dofs_j[k] = VolumeJunctionBaseUserObject::_flow_channel_dofs[c][j];
    }
  }
}
