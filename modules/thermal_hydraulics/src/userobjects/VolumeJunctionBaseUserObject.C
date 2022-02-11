//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunctionBaseUserObject.h"
#include "MooseVariableScalar.h"

InputParameters
VolumeJunctionBaseUserObject::validParams()
{
  InputParameters params = FlowJunctionUserObject::validParams();

  params.addRequiredParam<Real>("volume", "Volume of the junction");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "numerical_flux_names",
      "The names of the user objects that compute the numerical flux at each flow channel.");

  return params;
}

VolumeJunctionBaseUserObject::VolumeJunctionBaseUserObject(const InputParameters & params)
  : FlowJunctionUserObject(params),
    _volume(getParam<Real>("volume")),
    _numerical_flux_names(getParam<std::vector<UserObjectName>>("numerical_flux_names"))
{
  if (_numerical_flux_names.size() != _n_connections)
    mooseError(name(),
               ": The number of supplied numerical flux objects '",
               _numerical_flux_names.size(),
               "' does not match the number of connections '",
               _n_connections,
               "'.");
}

void
VolumeJunctionBaseUserObject::initialSetup()
{
  _n_flux_eq = _flow_variable_names.size();
  _n_scalar_eq = _scalar_variable_names.size();

  _scalar_dofs.resize(_n_scalar_eq);
  _flow_channel_dofs.resize(_n_connections);
  _phi_face_values.resize(_n_connections);
  for (auto && v : _phi_face_values)
    v.resize(_n_flux_eq);

  _residual.resize(_n_scalar_eq);
  _residual_jacobian_scalar_vars.resize(_n_scalar_eq);
  for (auto && v : _residual_jacobian_scalar_vars)
    v.resize(1, _n_scalar_eq);
  _residual_jacobian_flow_channel_vars.resize(_n_connections);
  for (auto && v : _residual_jacobian_flow_channel_vars)
    v.resize(_n_scalar_eq);
}

void
VolumeJunctionBaseUserObject::initialize()
{
  _flux.assign(_n_connections, std::vector<Real>(_n_flux_eq, 0.0));
  _flux_jacobian_scalar_vars.assign(_n_connections, DenseMatrix<Real>(_n_flux_eq, _n_scalar_eq));
  _flux_jacobian_flow_channel_vars.assign(_n_connections,
                                          DenseMatrix<Real>(_n_flux_eq, _n_flux_eq));

  std::fill(_residual.begin(), _residual.end(), 0.0);
  for (unsigned int i = 0; i < _n_scalar_eq; i++)
    _residual_jacobian_scalar_vars[i].zero();

  _connection_indices.clear();
}

void
VolumeJunctionBaseUserObject::storeConnectionData()
{
  // Get the connection index
  const unsigned int c = getBoundaryIDIndex();
  _connection_indices.push_back(c);

  // Get flow channel Dofs and basic function values
  _flow_channel_dofs[c].clear();
  for (unsigned int j = 0; j < _n_flux_eq; j++)
  {
    MooseVariable * var = getVar(_flow_variable_names[j], 0);

    auto && dofs = var->dofIndices();
    for (unsigned int k = 0; k < dofs.size(); k++)
      _flow_channel_dofs[c].push_back(dofs[k]);

    const VariablePhiValue & phi = var->phiFace();
    _phi_face_values[c][j].resize(phi.size());
    for (unsigned int k = 0; k < phi.size(); k++)
      _phi_face_values[c][j][k] = phi[k][0];
  }
}

void
VolumeJunctionBaseUserObject::execute()
{
  storeConnectionData();

  const unsigned int c = getBoundaryIDIndex();
  computeFluxesAndResiduals(c);
}

void
VolumeJunctionBaseUserObject::computeScalarJacobianWRTFlowDofs(const DenseMatrix<Real> & jac,
                                                               const unsigned int & c)
{
  for (unsigned int i = 0; i < _n_scalar_eq; i++)
  {
    _residual_jacobian_flow_channel_vars[c][i].resize(1, _flow_channel_dofs[c].size());

    unsigned int jk = 0;
    for (unsigned int j = 0; j < _n_flux_eq; j++)
    {
      for (unsigned int k = 0; k < _phi_face_values[c][j].size(); k++)
      {
        _residual_jacobian_flow_channel_vars[c][i](0, jk) = jac(i, j) * _phi_face_values[c][j][k];
        jk++;
      }
    }
  }
}

void
VolumeJunctionBaseUserObject::threadJoin(const UserObject & uo)
{
  const VolumeJunctionBaseUserObject & volume_junction_uo =
      dynamic_cast<const VolumeJunctionBaseUserObject &>(uo);

  // Store the data computed/retrieved in the other threads
  for (unsigned int i = 0; i < volume_junction_uo._connection_indices.size(); i++)
  {
    const unsigned int c = volume_junction_uo._connection_indices[i];

    _flux[c] = volume_junction_uo._flux[c];
    _flux_jacobian_scalar_vars[c] = volume_junction_uo._flux_jacobian_scalar_vars[c];
    _flux_jacobian_flow_channel_vars[c] = volume_junction_uo._flux_jacobian_flow_channel_vars[c];
    _flow_channel_dofs[c] = volume_junction_uo._flow_channel_dofs[c];
    _phi_face_values[c] = volume_junction_uo._phi_face_values[c];
    _residual_jacobian_flow_channel_vars[c] =
        volume_junction_uo._residual_jacobian_flow_channel_vars[c];
  }

  // Add the scalar residuals from the other threads
  for (unsigned int i = 0; i < _n_scalar_eq; i++)
  {
    _residual[i] += volume_junction_uo._residual[i];
    for (unsigned int j = 0; j < _n_scalar_eq; j++)
      _residual_jacobian_scalar_vars[i](0, j) +=
          volume_junction_uo._residual_jacobian_scalar_vars[i](0, j);
  }
}

void
VolumeJunctionBaseUserObject::finalize()
{
  FlowJunctionUserObject::finalize();

  // Get scalar Dofs
  for (unsigned int i = 0; i < _n_scalar_eq; i++)
  {
    auto var = getScalarVar(_scalar_variable_names[i], 0);
    auto && dofs = var->dofIndices();
    mooseAssert(dofs.size() == 1,
                "There should be exactly 1 coupled DoF index for the variable '" + var->name() +
                    "'.");
    _scalar_dofs[i] = dofs[0];
  }
}

const std::vector<Real> &
VolumeJunctionBaseUserObject::getResidual() const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  return _residual;
}

void
VolumeJunctionBaseUserObject::getScalarEquationJacobianData(const unsigned int & equation_index,
                                                            DenseMatrix<Real> & jacobian_block,
                                                            std::vector<dof_id_type> & dofs_i,
                                                            std::vector<dof_id_type> & dofs_j) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  // Store Jacobians w.r.t. scalar variables
  std::vector<Real> jacobian_block_entries(_n_scalar_eq);
  for (unsigned int j = 0; j < _n_scalar_eq; j++)
    jacobian_block_entries[j] = _residual_jacobian_scalar_vars[equation_index](0, j);
  dofs_i = {_scalar_dofs[equation_index]};
  dofs_j = _scalar_dofs;

  // Store Jacobian entries w.r.t. flow variables
  for (unsigned int c = 0; c < _n_connections; c++)
  {
    // Append entries
    const unsigned int n_old = jacobian_block_entries.size();
    const unsigned int n_add = _residual_jacobian_flow_channel_vars[c][equation_index].n();
    jacobian_block_entries.resize(n_old + n_add);
    for (unsigned int j = 0; j < n_add; j++)
      jacobian_block_entries[n_old + j] =
          _residual_jacobian_flow_channel_vars[c][equation_index](0, j);
    dofs_j.insert(dofs_j.end(), _flow_channel_dofs[c].begin(), _flow_channel_dofs[c].end());
  }

  jacobian_block.resize(1, dofs_j.size());
  for (unsigned int j = 0; j < dofs_j.size(); j++)
    jacobian_block(0, j) = jacobian_block_entries[j];
}

const std::vector<Real> &
VolumeJunctionBaseUserObject::getFlux(const unsigned int & connection_index) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  checkValidConnectionIndex(connection_index);
  return _flux[connection_index];
}

const DenseMatrix<Real> &
VolumeJunctionBaseUserObject::getFluxJacobianScalarVariables(
    const unsigned int & connection_index) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  checkValidConnectionIndex(connection_index);
  return _flux_jacobian_scalar_vars[connection_index];
}

const DenseMatrix<Real> &
VolumeJunctionBaseUserObject::getFluxJacobianFlowChannelVariables(
    const unsigned int & connection_index) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  checkValidConnectionIndex(connection_index);
  return _flux_jacobian_flow_channel_vars[connection_index];
}
