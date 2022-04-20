//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADShaftConnectableUserObjectInterface.h"
#include "MooseVariableScalar.h"
#include "UserObject.h"
#include "metaphysicl/parallel_numberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "libmesh/parallel_algebra.h"

InputParameters
ADShaftConnectableUserObjectInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

ADShaftConnectableUserObjectInterface::ADShaftConnectableUserObjectInterface(
    const MooseObject * moose_object)
  : _moose_object(moose_object), _n_shaft_eq(1)
{
  _omega_dof.resize(_n_shaft_eq);
}

void
ADShaftConnectableUserObjectInterface::initialize()
{
  _torque = 0;
  _moment_of_inertia = 0;
}

void
ADShaftConnectableUserObjectInterface::execute()
{
}

ADReal
ADShaftConnectableUserObjectInterface::getTorque() const
{
  return _torque;
}

ADReal
ADShaftConnectableUserObjectInterface::getMomentOfInertia() const
{
  return _moment_of_inertia;
}

void
ADShaftConnectableUserObjectInterface::setupConnections(unsigned int n_connections,
                                                        unsigned int n_flow_eq)
{
  _n_connections = n_connections;
  _n_flow_eq = n_flow_eq;
}

void
ADShaftConnectableUserObjectInterface::setConnectionData(
    const std::vector<std::vector<dof_id_type>> & flow_channel_dofs)
{
  _flow_channel_dofs = flow_channel_dofs;
}

void
ADShaftConnectableUserObjectInterface::setOmegaDofs(const MooseVariableScalar * omega_var)
{
  auto && dofs = omega_var->dofIndices();
  mooseAssert(dofs.size() == 1,
              "There should be exactly 1 coupled DoF index for the variable '" + omega_var->name() +
                  "'.");
  _omega_dof = dofs;
}

void
ADShaftConnectableUserObjectInterface::setupJunctionData(std::vector<dof_id_type> & scalar_dofs)
{
  _scalar_dofs = scalar_dofs;
}

void
ADShaftConnectableUserObjectInterface::finalize()
{
  _moose_object->comm().sum(_torque);
  _moose_object->comm().sum(_moment_of_inertia);
}

void
ADShaftConnectableUserObjectInterface::threadJoin(const UserObject & uo)
{
  const ADShaftConnectableUserObjectInterface & sctc_uo =
      dynamic_cast<const ADShaftConnectableUserObjectInterface &>(uo);
  _torque += sctc_uo._torque;
  _moment_of_inertia += sctc_uo._moment_of_inertia;
}
