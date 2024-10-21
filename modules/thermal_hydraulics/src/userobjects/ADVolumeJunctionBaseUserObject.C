//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVolumeJunctionBaseUserObject.h"
#include "MooseVariableScalar.h"
#include "FEProblemBase.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "libmesh/parallel_algebra.h"

InputParameters
ADVolumeJunctionBaseUserObject::validParams()
{
  InputParameters params = ADFlowJunctionUserObject::validParams();

  params.addParam<bool>(
      "use_scalar_variables", true, "True if the junction variables are scalar variables");
  params.addParam<subdomain_id_type>(
      "junction_subdomain_id",
      libMesh::invalid_uint,
      "Junction subdomain ID (required if 'use_scalar_variables' is 'false')");

  params.addRequiredParam<Real>("volume", "Volume of the junction");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "numerical_flux_names",
      "The names of the user objects that compute the numerical flux at each flow channel.");
  params.addClassDescription("User object to compute fluxes and residuals for a volume junction");
  return params;
}

ADVolumeJunctionBaseUserObject::ADVolumeJunctionBaseUserObject(const InputParameters & params)
  : ADFlowJunctionUserObject(params),
    _use_scalar_variables(getParam<bool>("use_scalar_variables")),
    _junction_subdomain_id(getParam<subdomain_id_type>("junction_subdomain_id")),
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

  if (!_use_scalar_variables && !isParamSetByUser("junction_subdomain_id"))
    mooseError("If 'use_scalar_variables' is set to false, 'junction_subdomain_id' is required.");
}

void
ADVolumeJunctionBaseUserObject::initialSetup()
{
  _n_flux_eq = _flow_variable_names.size();
  _n_scalar_eq = _scalar_variable_names.size();

  _scalar_dofs.resize(_n_scalar_eq);
  _cached_junction_var_values.resize(_n_scalar_eq);
  _flow_channel_dofs.resize(_n_connections);
  _residual.resize(_n_scalar_eq);
}

void
ADVolumeJunctionBaseUserObject::initialize()
{
  _flux.assign(_n_connections, std::vector<ADReal>(_n_flux_eq, 0.0));
  for (auto & i : _residual)
  {
    i.value() = 0;
    i.derivatives() = DNDerivativeType();
  }
  _connection_indices.clear();

  auto junction_vars = getJunctionVariables();

  // Cache the junction variable values and get their dof indices
  if (!_use_scalar_variables)
  {
    for (unsigned int i = 0; i < _n_scalar_eq; i++)
    {
      _cached_junction_var_values[i] = 0;
      _scalar_dofs[i] = 0;
    }

    for (const Elem * elem : *_mesh.getActiveLocalElementRange())
    {
      if (elem->subdomain_id() == _junction_subdomain_id)
      {
        // Reinitialize the element
        _fe_problem.setCurrentSubdomainID(elem, _tid);
        _fe_problem.prepare(elem, _tid);
        _fe_problem.reinitElem(elem, _tid);

        // Cache the junction variable values and Dof indices
        for (unsigned int i = 0; i < _n_scalar_eq; i++)
        {
          _cached_junction_var_values[i] = (*_junction_var_values[i])[0];

          auto && dofs = junction_vars[i]->dofIndices();
          mooseAssert(dofs.size() == 1,
                      "There should be exactly 1 coupled DoF index for the variable '" +
                          junction_vars[i]->name() + "'.");
          _scalar_dofs[i] = dofs[0];
        }
      }
    }

    comm().sum(_cached_junction_var_values);
    comm().sum(_scalar_dofs);
  }
  else
  {
    for (unsigned int i = 0; i < _junction_var_values.size(); i++)
      _cached_junction_var_values[i] = (*_junction_var_values[i])[0];

    for (unsigned int i = 0; i < _n_scalar_eq; i++)
    {
      auto && dofs = junction_vars[i]->dofIndices();
      mooseAssert(dofs.size() == 1,
                  "There should be exactly 1 coupled DoF index for the variable '" +
                      junction_vars[i]->name() + "'.");
      _scalar_dofs[i] = dofs[0];
    }
  }
}

void
ADVolumeJunctionBaseUserObject::storeConnectionData()
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
  }
}

void
ADVolumeJunctionBaseUserObject::execute()
{
  storeConnectionData();

  const unsigned int c = getBoundaryIDIndex();
  computeFluxesAndResiduals(c);
}

void
ADVolumeJunctionBaseUserObject::threadJoin(const UserObject & uo)
{
  const auto & volume_junction_uo = static_cast<const ADVolumeJunctionBaseUserObject &>(uo);

  // Store the data computed/retrieved in the other threads
  for (unsigned int i = 0; i < volume_junction_uo._connection_indices.size(); i++)
  {
    const unsigned int c = volume_junction_uo._connection_indices[i];

    _flux[c] = volume_junction_uo._flux[c];
    _flow_channel_dofs[c] = volume_junction_uo._flow_channel_dofs[c];
  }

  // Add the scalar residuals from the other threads
  for (unsigned int i = 0; i < _n_scalar_eq; i++)
    _residual[i] += volume_junction_uo._residual[i];
}

const std::vector<ADReal> &
ADVolumeJunctionBaseUserObject::getResidual() const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  return _residual;
}

const std::vector<ADReal> &
ADVolumeJunctionBaseUserObject::getFlux(const unsigned int & connection_index) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  checkValidConnectionIndex(connection_index);
  return _flux[connection_index];
}

std::vector<const MooseVariableBase *>
ADVolumeJunctionBaseUserObject::getJunctionVariables() const
{
  std::vector<const MooseVariableBase *> vars(_scalar_variable_names.size());
  for (unsigned int i = 0; i < _scalar_variable_names.size(); i++)
    vars[i] = getJunctionVar(_scalar_variable_names[i], 0);
  return vars;
}

const MooseVariableBase *
ADVolumeJunctionBaseUserObject::getJunctionVar(const std::string & var_name, unsigned int i) const
{
  const MooseVariableBase * var;
  if (_use_scalar_variables)
    var = getScalarVar(var_name, i);
  else
    var = getVar(var_name, i);
  return var;
}

const ADVariableValue &
ADVolumeJunctionBaseUserObject::coupledJunctionValue(const std::string & var_name,
                                                     unsigned int i) const
{
  return _use_scalar_variables ? adCoupledScalarValue(var_name, i) : adCoupledValue(var_name, i);
}
