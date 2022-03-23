//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/dense_matrix.h"
#include "InputParameters.h"

using namespace libMesh;
class UserObject;
class MooseVariableScalar;

/**
 * Interface class for user objects that are connected to a shaft
 */
class ADShaftConnectableUserObjectInterface
{
public:
  ADShaftConnectableUserObjectInterface(const MooseObject * moose_object);

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & uo);

  virtual ADReal getTorque() const;
  virtual ADReal getMomentOfInertia() const;

protected:
  virtual void setupConnections(unsigned int n_connections, unsigned int n_flow_eq);
  /**
   * Stores data computed by a volume-junction-like object associated with the conection
   */
  virtual void setConnectionData(const std::vector<std::vector<dof_id_type>> & flow_channel_dofs);
  virtual void setOmegaDofs(const MooseVariableScalar * omega_var);
  /**
   * Stores data associated with a junction component
   */
  virtual void setupJunctionData(std::vector<dof_id_type> & scalar_dofs);

  /// Associated MOOSE object
  const MooseObject * _moose_object;
  /// Number of equation in the shaft component
  unsigned int _n_shaft_eq;
  /// Number of flow channels the shaft connected component is attached to
  unsigned int _n_connections;
  /// Number of flow variables in connected flow channels
  unsigned int _n_flow_eq;

  /// Degrees of freedom for omega variable (from shaft)
  std::vector<dof_id_type> _omega_dof;
  /// Degrees of freedom for scalar variables (from junction)
  std::vector<dof_id_type> _scalar_dofs;
  /// Degrees of freedom for flow channel variables, for each connection
  std::vector<std::vector<dof_id_type>> _flow_channel_dofs;
  /// Total torque
  ADReal _torque;
  /// Moment of inertia
  ADReal _moment_of_inertia;

public:
  static InputParameters validParams();
};
