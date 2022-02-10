//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectedTestComponentUserObject.h"
#include "MooseVariableScalar.h"

registerMooseObject("ThermalHydraulicsTestApp", ShaftConnectedTestComponentUserObject);

InputParameters
ShaftConnectedTestComponentUserObject::validParams()
{
  InputParameters params = VolumeJunctionBaseUserObject::validParams();
  params += ShaftConnectableUserObjectInterface::validParams();
  params.addRequiredCoupledVar("rhoA", "rho*A of the connected flow channels");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the connected flow channels");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the connected flow channels");
  params.addRequiredCoupledVar("jct_var", "Junction scalar variable");
  params.addRequiredCoupledVar("omega", "Shaft speed scalar variable");
  params.addClassDescription(
      "Test component for showing how to connect a junction-derived object to a shaft");
  return params;
}

ShaftConnectedTestComponentUserObject::ShaftConnectedTestComponentUserObject(
    const InputParameters & params)
  : VolumeJunctionBaseUserObject(params),
    ShaftConnectableUserObjectInterface(this),
    _rhoA(coupledValue("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _rhoEA(coupledValue("rhoEA")),

    _jct_var(coupledScalarValue("jct_var")),
    _omega(coupledScalarValue("omega"))
{
  _flow_variable_names.resize(3);
  _flow_variable_names[0] = "rhoA";
  _flow_variable_names[1] = "rhouA";
  _flow_variable_names[2] = "rhoEA";

  unsigned int n_jct_vars = 1;
  _scalar_variable_names.resize(n_jct_vars);
  _scalar_variable_names[0] = "jct_var";

  _torque_jacobian_scalar_vars.resize(_n_shaft_eq, n_jct_vars);
  _moi_jacobian_scalar_vars.resize(_n_shaft_eq, n_jct_vars);
}

void
ShaftConnectedTestComponentUserObject::initialize()
{
  VolumeJunctionBaseUserObject::initialize();
  ShaftConnectableUserObjectInterface::initialize();
  for (unsigned int i = 0; i < _n_scalar_eq; i++)
    _residual_jacobian_omega_var[i].zero();
}

void
ShaftConnectedTestComponentUserObject::initialSetup()
{
  VolumeJunctionBaseUserObject::initialSetup();

  ShaftConnectableUserObjectInterface::setupConnections(
      VolumeJunctionBaseUserObject::_n_connections, VolumeJunctionBaseUserObject::_n_flux_eq);

  _residual_jacobian_omega_var.resize(_n_scalar_eq);
  for (auto && v : _residual_jacobian_omega_var)
    v.resize(1, ShaftConnectableUserObjectInterface::_n_shaft_eq);
}

void
ShaftConnectedTestComponentUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  Real tau_c = 0;
  Real moi_c = 0;

  if (c == 0)
  {
    tau_c = _jct_var[0] + 2. * _omega[0];
    Real dtau_c_djct_var = 1.;
    Real dtau_c_domega = 2.;

    moi_c = 2. * _jct_var[0] + 3. * _omega[0];
    Real dmoi_djct_var = 2.;
    Real dmoi_domega = 3.;

    // residual for the junction variable
    _residual[0] += tau_c + moi_c;
    // djct_var_djct_var
    _residual_jacobian_scalar_vars[0](0, 0) += dtau_c_djct_var + dmoi_djct_var;
    // djct_var_domega
    _residual_jacobian_omega_var[0](0, 0) += dtau_c_domega + dmoi_domega;

    _torque += tau_c;
    _torque_jacobian_scalar_vars(0, 0) = dtau_c_djct_var;
    _torque_jacobian_omega_var(0, 0) = dtau_c_domega;

    _moment_of_inertia += moi_c;
    _moi_jacobian_scalar_vars(0, 0) = dmoi_djct_var;
    _moi_jacobian_omega_var(0, 0) = dmoi_domega;
  }

  tau_c = (c + 1) * _rhoA[0] + (c + 2) * _rhouA[0] + (c + 3) * _rhoEA[0];
  moi_c = (c + 7) * _rhoA[0] + (c + 8) * _rhouA[0] + (c + 9) * _rhoEA[0];

  _residual[0] += tau_c + moi_c;
  // djct_var_dUi (i.e. wrt flow variables)
  {
    DenseMatrix<Real> jac(1, 3);
    // djct_var_drhoA
    jac(0, 0) = (c + 1) + (c + 7);
    // djct_var_drhouA
    jac(0, 1) = (c + 2) + (c + 8);
    // djct_var_drhoEA
    jac(0, 2) = (c + 3) + (c + 9);

    computeScalarJacobianWRTFlowDofs(jac, c);
  }

  _torque += tau_c;
  // dtorque_dUi (i.e. wrt flow variables)
  {
    DenseMatrix<Real> jac(1, 3);
    // dtorque_drhoA
    jac(0, 0) = (c + 1);
    // dtorque_drhouA
    jac(0, 1) = (c + 2);
    // dtorque_drhoEA
    jac(0, 2) = (c + 3);

    computeTorqueScalarJacobianWRTFlowDofs(jac, c);
  }

  _moment_of_inertia += moi_c;
  // dmoi_dUi (i.e. wrt flow variables)
  {
    DenseMatrix<Real> jac(1, 3);
    // dmoi_drhoA
    jac(0, 0) = (c + 7);
    // dmoi_drhouA
    jac(0, 1) = (c + 8);
    // dmoi_drhoEA
    jac(0, 2) = (c + 9);

    computeMomentOfInertiaScalarJacobianWRTFlowDofs(jac, c);
  }
}

void
ShaftConnectedTestComponentUserObject::execute()
{
  VolumeJunctionBaseUserObject::storeConnectionData();
  ShaftConnectableUserObjectInterface::setConnectionData(
      VolumeJunctionBaseUserObject::_phi_face_values,
      VolumeJunctionBaseUserObject::_flow_channel_dofs);

  const unsigned int c = getBoundaryIDIndex();
  computeFluxesAndResiduals(c);
}

void
ShaftConnectedTestComponentUserObject::finalize()
{
  VolumeJunctionBaseUserObject::finalize();

  ShaftConnectableUserObjectInterface::setupJunctionData(
      VolumeJunctionBaseUserObject::_scalar_dofs);
  ShaftConnectableUserObjectInterface::setOmegaDofs(getScalarVar("omega", 0));
}

void
ShaftConnectedTestComponentUserObject::threadJoin(const UserObject & uo)
{
  VolumeJunctionBaseUserObject::threadJoin(uo);
  ShaftConnectableUserObjectInterface::threadJoin(uo);
}

void
ShaftConnectedTestComponentUserObject::getScalarEquationJacobianData(
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
