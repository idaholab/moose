//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectedPump1PhaseUserObject.h"
#include "ShaftConnectedPump1Phase.h"
#include "VolumeJunction1Phase.h"
#include "MooseVariableScalar.h"
#include "THMIndices3Eqn.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedPump1PhaseUserObject);

InputParameters
ShaftConnectedPump1PhaseUserObject::validParams()
{
  InputParameters params = VolumeJunction1PhaseUserObject::validParams();
  params += ShaftConnectableUserObjectInterface::validParams();

  params.addParam<BoundaryName>("inlet", "Pump inlet");
  params.addParam<BoundaryName>("outlet", "Pump outlet");
  params.addRequiredParam<Point>("di_out", "Direction of connected outlet");
  params.addRequiredParam<Real>("gravity_magnitude", "Gravity constant, [m/s^2]");
  params.addRequiredParam<Real>("omega_rated", "Rated pump speed [rad/s]");
  params.addRequiredParam<Real>("volumetric_rated", "Rated pump volumetric flow rate [m^3/s]");
  params.addRequiredParam<Real>("head_rated", "Rated pump head [m]");
  params.addRequiredParam<Real>("torque_rated", "Rated pump torque [N-m]");
  params.addRequiredParam<Real>("density_rated", "Rated pump fluid density [kg/m^3]");
  params.addRequiredParam<Real>("speed_cr_fr", "Pump speed threshold for friction [-]");
  params.addRequiredParam<Real>("tau_fr_const", "Pump friction constant [N-m]");
  params.addRequiredParam<std::vector<Real>>("tau_fr_coeff", "Friction coefficients [N-m]");
  params.addRequiredParam<Real>("speed_cr_I", "Pump speed threshold for inertia [-]");
  params.addRequiredParam<Real>("inertia_const", "Pump inertia constant [kg-m^2]");
  params.addRequiredParam<std::vector<Real>>("inertia_coeff", "Pump inertia coefficients [kg-m^2]");
  params.addRequiredParam<FunctionName>("head", "Function to compute data for pump head [-]");
  params.addRequiredParam<FunctionName>("torque_hydraulic",
                                        "Function to compute data for pump torque [-]");
  params.addRequiredParam<std::string>("pump_name", "Name of the instance of this pump component");
  params.addRequiredCoupledVar("omega", "Shaft rotational speed [rad/s]");

  params.addClassDescription(
      "Computes and caches flux and residual vectors for a 1-phase pump. Also computes pump torque "
      "and head which is passed to the connected shaft");

  return params;
}

ShaftConnectedPump1PhaseUserObject::ShaftConnectedPump1PhaseUserObject(
    const InputParameters & params)
  : VolumeJunction1PhaseUserObject(params),
    ShaftConnectableUserObjectInterface(this),

    _di_out(getParam<Point>("di_out")),
    _g(getParam<Real>("gravity_magnitude")),
    _omega_rated(getParam<Real>("omega_rated")),
    _volumetric_rated(getParam<Real>("volumetric_rated")),
    _head_rated(getParam<Real>("head_rated")),
    _torque_rated(getParam<Real>("torque_rated")),
    _density_rated(getParam<Real>("density_rated")),
    _speed_cr_fr(getParam<Real>("speed_cr_fr")),
    _tau_fr_const(getParam<Real>("tau_fr_const")),
    _tau_fr_coeff(getParam<std::vector<Real>>("tau_fr_coeff")),
    _speed_cr_I(getParam<Real>("speed_cr_I")),
    _inertia_const(getParam<Real>("inertia_const")),
    _inertia_coeff(getParam<std::vector<Real>>("inertia_coeff")),
    _head(getFunction("head")),
    _torque_hydraulic(getFunction("torque_hydraulic")),
    _pump_name(getParam<std::string>("pump_name")),
    _omega(coupledScalarValue("omega"))
{
  unsigned int n_jct_vars = _scalar_variable_names.size();
  _torque_jacobian_scalar_vars.resize(_n_shaft_eq, n_jct_vars);
  _moi_jacobian_scalar_vars.resize(_n_shaft_eq, n_jct_vars);
  _moi_jacobian_scalar_vars.zero();
}

void
ShaftConnectedPump1PhaseUserObject::initialSetup()
{
  VolumeJunctionBaseUserObject::initialSetup();

  ShaftConnectableUserObjectInterface::setupConnections(
      VolumeJunctionBaseUserObject::_n_connections, VolumeJunctionBaseUserObject::_n_flux_eq);

  _residual_jacobian_omega_var.resize(_n_scalar_eq);
  for (auto && v : _residual_jacobian_omega_var)
    v.resize(1, ShaftConnectableUserObjectInterface::_n_shaft_eq);
}

void
ShaftConnectedPump1PhaseUserObject::initialize()
{
  VolumeJunctionBaseUserObject::initialize();
  ShaftConnectableUserObjectInterface::initialize();
  for (unsigned int i = 0; i < _n_scalar_eq; i++)
    _residual_jacobian_omega_var[i].zero();

  _hydraulic_torque = 0;
  _friction_torque = 0;
  _pump_head = 0;
}

void
ShaftConnectedPump1PhaseUserObject::execute()
{
  VolumeJunctionBaseUserObject::storeConnectionData();
  ShaftConnectableUserObjectInterface::setConnectionData(
      VolumeJunctionBaseUserObject::_phi_face_values,
      VolumeJunctionBaseUserObject::_flow_channel_dofs);

  const unsigned int c = getBoundaryIDIndex();

  computeFluxesAndResiduals(c);
}

void
ShaftConnectedPump1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  VolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  // inlet c=0 established in component
  if (c == 0)
  {
    Real alpha = _omega[0] / _omega_rated;
    Real dalpha_domega = 1 / _omega_rated;

    Real Q_in = (_rhouA[0] / _rhoA[0]) * _A_ref;
    Real dQ_in_drhoA = -_rhouA[0] * _A_ref / (_rhoA[0] * _rhoA[0]);
    Real dQ_in_drhouA = _A_ref / _rhoA[0];

    Real nu = Q_in / _volumetric_rated;
    Real dv_drhoA = dQ_in_drhoA / _volumetric_rated;
    Real dv_drhouA = dQ_in_drhouA / _volumetric_rated;

    Real c_coef;
    if ((alpha >= 0) & (nu >= 0))
      c_coef = 0;
    else if ((alpha > 0) & (nu < 0))
      c_coef = libMesh::pi;
    else if ((alpha <= 0) & (nu <= 0))
      c_coef = libMesh::pi;
    else if ((alpha < 0) & (nu > 0))
      c_coef = 2 * libMesh::pi;
    else
    {
      mooseError(_pump_name, ": The pump is outside normal operating regime.");
    }

    // Head and torque
    Real x_p = c_coef + std::atan(alpha / nu);
    Real dx_p_dalpha = nu / (alpha * alpha + nu * nu);
    Real dx_p_dnu = -alpha / (alpha * alpha + nu * nu);

    Real dx_p_domega = dx_p_dalpha * dalpha_domega;
    Real dx_p_drhoA = dx_p_dnu * dv_drhoA;
    Real dx_p_drhouA = dx_p_dnu * dv_drhouA;

    Real wt = _torque_hydraulic.value(x_p, Point());
    Real dwt_dx = _torque_hydraulic.timeDerivative(x_p, Point());
    Real dwt_domega = dwt_dx * dx_p_domega;
    Real dwt_drhoA = dwt_dx * dx_p_drhoA;
    Real dwt_drhouA = dwt_dx * dx_p_drhouA;

    Real wh = _head.value(x_p, Point());
    Real dwh_dx = _head.timeDerivative(x_p, Point());
    Real dwh_domega = dwh_dx * dx_p_domega;
    Real dwh_drhoA = dwh_dx * dx_p_drhoA;
    Real dwh_drhouA = dwh_dx * dx_p_drhouA;

    Real y = alpha * alpha + nu * nu;
    Real dy_domega = 2. * alpha * dalpha_domega;
    Real dy_drhoA = 2. * nu * dv_drhoA;
    Real dy_drhouA = 2. * nu * dv_drhouA;

    Real zt = wt * _torque_rated;
    Real dzt_domega = dwt_domega * _torque_rated;
    Real dzt_drhoA = dwt_drhoA * _torque_rated;
    Real dzt_drhouA = dwt_drhouA * _torque_rated;

    // Real homologous_torque = -(alpha * alpha + nu * nu) * wt * _torque_rated;
    Real homologous_torque = -y * zt;
    _hydraulic_torque = homologous_torque * ((_rhoV[0] / _volume) / _density_rated);
    Real dtau_hyd_drhoV = homologous_torque * ((1.0 / _volume) / _density_rated);
    Real dtau_hyd_domega =
        -(dy_domega * zt + y * dzt_domega) * ((_rhoV[0] / _volume) / _density_rated);
    Real dtau_hyd_drhoA =
        -(dy_drhoA * zt + y * dzt_drhoA) * ((_rhoV[0] / _volume) / _density_rated);
    Real dtau_hyd_drhouA =
        -(dy_drhouA * zt + y * dzt_drhouA) * ((_rhoV[0] / _volume) / _density_rated);

    Real zh = wh * _head_rated;
    Real dzh_domega = dwh_domega * _head_rated;
    Real dzh_drhoA = dwh_drhoA * _head_rated;
    Real dzh_drhouA = dwh_drhouA * _head_rated;

    // _pump_head = (alpha * alpha + nu * nu) * wh * _head_rated;
    _pump_head = y * zh;
    Real dhead_domega = dy_domega * zh + y * dzh_domega;
    Real dhead_drhoA = dy_drhoA * zh + y * dzh_drhoA;
    Real dhead_drhouA = dy_drhouA * zh + y * dzh_drhouA;

    // MoI
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

    // friction torque
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

    // compute momentum and energy source terms
    // a negative torque value results in a positive S_energy
    const Real S_energy = -(_hydraulic_torque + _friction_torque) * _omega[0];
    const Real dS_energy_drhoA = -dtau_hyd_drhoA * _omega[0];
    const Real dS_energy_drhouA = -dtau_hyd_drhouA * _omega[0];
    const Real dS_energy_drhoV = -dtau_hyd_drhoV * _omega[0];
    const Real dS_energy_domega =
        -(_hydraulic_torque + _friction_torque) - (dtau_hyd_domega + dtau_fr_domega) * _omega[0];

    // a positive head value results in a positive S_momentum
    const RealVectorValue S_momentum = (_rhoV[0] / _volume) * _g * _pump_head * _A_ref * _di_out;
    //
    const RealVectorValue dS_momentum_drhoV = (1 / _volume) * _g * _pump_head * _A_ref * _di_out;
    const RealVectorValue dS_momentum_domega =
        (_rhoV[0] / _volume) * _g * dhead_domega * _A_ref * _di_out;
    const RealVectorValue dS_momentum_drhoA =
        (_rhoV[0] / _volume) * _g * dhead_drhoA * _A_ref * _di_out;
    const RealVectorValue dS_momentum_drhouA =
        (_rhoV[0] / _volume) * _g * dhead_drhouA * _A_ref * _di_out;

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
    for (unsigned int i = 0; i < 3; i++)
    {
      _residual_jacobian_scalar_vars[rhovelV_index(i)](0, VolumeJunction1Phase::RHOV_INDEX) -=
          dS_momentum_drhoV(i);
      _residual_jacobian_omega_var[rhovelV_index(i)](0, 0) -= dS_momentum_domega(i);
    }

    {
      DenseMatrix<Real> jac(_n_scalar_eq, _n_flux_eq);
      jac(VolumeJunction1Phase::RHOEV_INDEX, 0) -= dS_energy_drhoA;
      jac(VolumeJunction1Phase::RHOEV_INDEX, 1) -= dS_energy_drhouA;
      jac(VolumeJunction1Phase::RHOUV_INDEX, 0) -= dS_momentum_drhoA(0);
      jac(VolumeJunction1Phase::RHOUV_INDEX, 1) -= dS_momentum_drhouA(0);
      jac(VolumeJunction1Phase::RHOVV_INDEX, 0) -= dS_momentum_drhoA(1);
      jac(VolumeJunction1Phase::RHOVV_INDEX, 1) -= dS_momentum_drhouA(1);
      jac(VolumeJunction1Phase::RHOWV_INDEX, 0) -= dS_momentum_drhoA(2);
      jac(VolumeJunction1Phase::RHOWV_INDEX, 1) -= dS_momentum_drhouA(2);
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

    _torque += _hydraulic_torque + _friction_torque;
    _torque_jacobian_omega_var(0, 0) = dtau_hyd_domega + dtau_fr_domega;
    _torque_jacobian_scalar_vars.zero();
    _torque_jacobian_scalar_vars(0, VolumeJunction1Phase::RHOV_INDEX) = dtau_hyd_drhoV;

    _moi_jacobian_omega_var(0, 0) = dmoi_domega;

    // dtau_dUi (i.e. wrt flow variables)
    {
      DenseMatrix<Real> jac(1, THM3Eqn::N_EQ);
      jac.zero();
      jac(0, THM3Eqn::EQ_MASS) = dtau_hyd_drhoA;
      jac(0, THM3Eqn::EQ_MOMENTUM) = dtau_hyd_drhouA;
      computeTorqueScalarJacobianWRTFlowDofs(jac, c);
    }
  }
  else
  {
    // dtau_dUi (i.e. wrt flow variables)
    {
      DenseMatrix<Real> jac(1, THM3Eqn::N_EQ);
      jac.zero();
      computeTorqueScalarJacobianWRTFlowDofs(jac, c);
    }
  }

  // dmoi_dUi (i.e. wrt flow variables)
  {
    DenseMatrix<Real> jac(1, THM3Eqn::N_EQ);
    jac.zero();
    computeMomentOfInertiaScalarJacobianWRTFlowDofs(jac, c);
  }
}

Real
ShaftConnectedPump1PhaseUserObject::getHydraulicTorque() const
{
  return _hydraulic_torque;
}

Real
ShaftConnectedPump1PhaseUserObject::getFrictionTorque() const
{
  return _friction_torque;
}

Real
ShaftConnectedPump1PhaseUserObject::getPumpHead() const
{
  return _pump_head;
}

void
ShaftConnectedPump1PhaseUserObject::finalize()
{
  VolumeJunctionBaseUserObject::finalize();

  ShaftConnectableUserObjectInterface::setupJunctionData(
      VolumeJunctionBaseUserObject::_scalar_dofs);
  ShaftConnectableUserObjectInterface::setOmegaDofs(getScalarVar("omega", 0));
}

void
ShaftConnectedPump1PhaseUserObject::threadJoin(const UserObject & uo)
{
  VolumeJunctionBaseUserObject::threadJoin(uo);
  ShaftConnectableUserObjectInterface::threadJoin(uo);

  const ShaftConnectedPump1PhaseUserObject & scpuo =
      dynamic_cast<const ShaftConnectedPump1PhaseUserObject &>(uo);
  _hydraulic_torque += scpuo._hydraulic_torque;
  _friction_torque += scpuo._friction_torque;
  _pump_head += scpuo._pump_head;

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
ShaftConnectedPump1PhaseUserObject::getScalarEquationJacobianData(
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
