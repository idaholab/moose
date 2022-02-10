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
class ShaftConnectableUserObjectInterface
{
public:
  ShaftConnectableUserObjectInterface(const MooseObject * moose_object);

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & uo);

  virtual Real getTorque() const;
  virtual void getTorqueJacobianData(DenseMatrix<Real> & jacobian_block,
                                     std::vector<dof_id_type> & dofs_j) const;
  virtual Real getMomentOfInertia() const;
  virtual void getMomentOfInertiaJacobianData(DenseMatrix<Real> & jacobian_block,
                                              std::vector<dof_id_type> & dofs_j) const;

protected:
  virtual void setupConnections(unsigned int n_connections, unsigned int n_flow_eq);
  /**
   * Stores data computed by a volume-junction-like object associated with the conection
   */
  virtual void
  setConnectionData(const std::vector<std::vector<std::vector<Real>>> & phi_face_values,
                    const std::vector<std::vector<dof_id_type>> & flow_channel_dofs);
  virtual void setOmegaDofs(const MooseVariableScalar * omega_var);
  /**
   * Stores data associated with a junction component
   */
  virtual void setupJunctionData(std::vector<dof_id_type> & scalar_dofs);

  virtual void computeMomentOfInertiaScalarJacobianWRTFlowDofs(const DenseMatrix<Real> & jac,
                                                               const unsigned int & c);
  virtual void computeTorqueScalarJacobianWRTFlowDofs(const DenseMatrix<Real> & jac,
                                                      const unsigned int & c);

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
  Real _torque;
  /// Moment of inertia
  Real _moment_of_inertia;

  /// Cached scalar residual Jacobian matrices w.r.t. flow channel variables,
  /// for each connection (first index) and each scalar variable (second index)
  std::vector<DenseMatrix<Real>> _torque_jacobian_flow_channel_vars;
  /// Jacobian entries of torque wrt to scalar variables (from junction)
  DenseMatrix<Real> _torque_jacobian_scalar_vars;
  /// Jacobian entries of torque wrt to omega variable (from shaft)
  DenseMatrix<Real> _torque_jacobian_omega_var;

  std::vector<DenseMatrix<Real>> _moi_jacobian_flow_channel_vars;
  /// Jacobian entries of moment of inertia wrt to omega scalar variables (from junction)
  DenseMatrix<Real> _moi_jacobian_scalar_vars;
  /// Jacobian entries of moment of inertia wrt to omega variable (from shaft)
  DenseMatrix<Real> _moi_jacobian_omega_var;

  /// Side shape function value (i.e. side from the flow channels)
  std::vector<std::vector<std::vector<Real>>> _phi_face_values;

public:
  static InputParameters validParams();
};
