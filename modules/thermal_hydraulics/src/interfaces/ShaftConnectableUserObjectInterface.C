//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectableUserObjectInterface.h"
#include "MooseVariableScalar.h"
#include "UserObject.h"

InputParameters
ShaftConnectableUserObjectInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

ShaftConnectableUserObjectInterface::ShaftConnectableUserObjectInterface(
    const MooseObject * /*moose_object*/)
  : _n_shaft_eq(1)
{
  _omega_dof.resize(_n_shaft_eq);
  _torque_jacobian_omega_var.resize(1, _n_shaft_eq);
  _moi_jacobian_omega_var.resize(1, _n_shaft_eq);
}

void
ShaftConnectableUserObjectInterface::initialize()
{
  _torque = 0;
  _moment_of_inertia = 0;
}

void
ShaftConnectableUserObjectInterface::execute()
{
}

Real
ShaftConnectableUserObjectInterface::getTorque() const
{
  return _torque;
}

void
ShaftConnectableUserObjectInterface::getTorqueJacobianData(DenseMatrix<Real> & jacobian_block,
                                                           std::vector<dof_id_type> & dofs_j) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  unsigned int n_dofs = 1 + _scalar_dofs.size();
  for (unsigned int c = 0; c < _n_connections; c++)
    n_dofs += _torque_jacobian_flow_channel_vars[c].n();

  jacobian_block.resize(_n_shaft_eq, n_dofs);
  dofs_j.resize(n_dofs);

  unsigned int k = 0;
  jacobian_block(0, k) = _torque_jacobian_omega_var(0, 0);
  dofs_j[k] = _omega_dof[0];
  k++;

  // Store Jacobian entries w.r.t. scalar variables
  for (unsigned int j = 0; j < _scalar_dofs.size(); j++, k++)
  {
    jacobian_block(0, k) = _torque_jacobian_scalar_vars(0, j);
    dofs_j[k] = _scalar_dofs[j];
  }
  // Store Jacobian entries w.r.t. flow variables
  for (unsigned int c = 0; c < _n_connections; c++)
  {
    for (unsigned int j = 0; j < _torque_jacobian_flow_channel_vars[c].n(); j++, k++)
    {
      jacobian_block(0, k) = _torque_jacobian_flow_channel_vars[c](0, j);
      dofs_j[k] = _flow_channel_dofs[c][j];
    }
  }
}

Real
ShaftConnectableUserObjectInterface::getMomentOfInertia() const
{
  return _moment_of_inertia;
}

void
ShaftConnectableUserObjectInterface::getMomentOfInertiaJacobianData(
    DenseMatrix<Real> & jacobian_block, std::vector<dof_id_type> & dofs_j) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  unsigned int n_dofs = 1 + _scalar_dofs.size();
  for (unsigned int c = 0; c < _n_connections; c++)
    n_dofs += _moi_jacobian_flow_channel_vars[c].n();

  jacobian_block.resize(_n_shaft_eq, n_dofs);
  dofs_j.resize(n_dofs);

  unsigned int k = 0;
  jacobian_block(0, k) = _moi_jacobian_omega_var(0, 0);
  dofs_j[k] = _omega_dof[0];
  k++;

  // Store Jacobian entries w.r.t. scalar variables
  for (unsigned int j = 0; j < _scalar_dofs.size(); j++, k++)
  {
    jacobian_block(0, k) = _moi_jacobian_scalar_vars(0, j);
    dofs_j[k] = _scalar_dofs[j];
  }
  // Store Jacobian entries w.r.t. flow variables
  for (unsigned int c = 0; c < _n_connections; c++)
  {
    for (unsigned int j = 0; j < _moi_jacobian_flow_channel_vars[c].n(); j++, k++)
    {
      jacobian_block(0, k) = _moi_jacobian_flow_channel_vars[c](0, j);
      dofs_j[k] = _flow_channel_dofs[c][j];
    }
  }
}

void
ShaftConnectableUserObjectInterface::setupConnections(unsigned int n_connections,
                                                      unsigned int n_flow_eq)
{
  _n_connections = n_connections;
  _torque_jacobian_flow_channel_vars.resize(_n_connections);
  _moi_jacobian_flow_channel_vars.resize(_n_connections);

  _n_flow_eq = n_flow_eq;
}

void
ShaftConnectableUserObjectInterface::setConnectionData(
    const std::vector<std::vector<std::vector<Real>>> & phi_face_values,
    const std::vector<std::vector<dof_id_type>> & flow_channel_dofs)
{
  _flow_channel_dofs = flow_channel_dofs;
  _phi_face_values = phi_face_values;
}

void
ShaftConnectableUserObjectInterface::setOmegaDofs(const MooseVariableScalar * omega_var)
{
  auto && dofs = omega_var->dofIndices();
  mooseAssert(dofs.size() == 1,
              "There should be exactly 1 coupled DoF index for the variable '" + omega_var->name() +
                  "'.");
  _omega_dof = dofs;
}

void
ShaftConnectableUserObjectInterface::setupJunctionData(std::vector<dof_id_type> & scalar_dofs)
{
  _scalar_dofs = scalar_dofs;
}

void
ShaftConnectableUserObjectInterface::computeTorqueScalarJacobianWRTFlowDofs(
    const DenseMatrix<Real> & jac, const unsigned int & c)
{
  _torque_jacobian_flow_channel_vars[c].resize(1, _flow_channel_dofs[c].size());
  unsigned int jk = 0;
  for (unsigned int j = 0; j < _n_flow_eq; j++)
  {
    for (unsigned int k = 0; k < _phi_face_values[c][j].size(); k++)
    {
      _torque_jacobian_flow_channel_vars[c](0, jk) = jac(0, j) * _phi_face_values[c][j][k];
      jk++;
    }
  }
}

void
ShaftConnectableUserObjectInterface::computeMomentOfInertiaScalarJacobianWRTFlowDofs(
    const DenseMatrix<Real> & jac, const unsigned int & c)
{
  _moi_jacobian_flow_channel_vars[c].resize(1, _flow_channel_dofs[c].size());
  unsigned int jk = 0;
  for (unsigned int j = 0; j < _n_flow_eq; j++)
  {
    for (unsigned int k = 0; k < _phi_face_values[c][j].size(); k++)
    {
      _moi_jacobian_flow_channel_vars[c](0, jk) = jac(0, j) * _phi_face_values[c][j][k];
      jk++;
    }
  }
}

void
ShaftConnectableUserObjectInterface::finalize()
{
}

void
ShaftConnectableUserObjectInterface::threadJoin(const UserObject & uo)
{
  const ShaftConnectableUserObjectInterface & sctc_uo =
      dynamic_cast<const ShaftConnectableUserObjectInterface &>(uo);
  _torque += sctc_uo._torque;
  _moment_of_inertia += sctc_uo._moment_of_inertia;
}
