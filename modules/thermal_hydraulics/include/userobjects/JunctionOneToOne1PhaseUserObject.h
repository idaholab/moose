#pragma once

#include "FlowJunctionUserObject.h"

class NumericalFlux3EqnBase;

/**
 * Computes flux between two subdomains for 1-phase one-to-one junction
 */
class JunctionOneToOne1PhaseUserObject : public FlowJunctionUserObject
{
public:
  JunctionOneToOne1PhaseUserObject(const InputParameters & params);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

  const std::vector<Real> & getFlux(const unsigned int & connection_index) const override;

  /**
   * Gets the Jacobian entries for an equation w.r.t. to another equation
   *
   * This includes both subdomains, so there should be two Jacobian entries
   * returned, one for each subdomain.
   *
   * @param[in] connection_index   Index for the connection
   * @param[in] equation_i   Index for the residual equation
   * @param[in] equation_j   Index for the derivative equation
   * @param[out] jacobian_block   Dense matrix storage of the returned Jacobian entries
   * @param[out] dofs_j   Vector of DoF indices corresponding to Jacobian entries
   */
  void getJacobianEntries(const unsigned int & connection_index,
                          const unsigned int & equation_i,
                          const unsigned int & equation_j,
                          DenseMatrix<Real> & jacobian_block,
                          std::vector<dof_id_type> & dofs_j) const;

protected:
  /// Cross-sectional area of connected flow channels
  const VariableValue & _A;
  /// rho*A of the connected flow channels
  const VariableValue & _rhoA;
  /// rho*u*A of the connected flow channels
  const VariableValue & _rhouA;
  /// rho*E*A of the connected flow channels
  const VariableValue & _rhoEA;

  /// Numerical flux user object
  const NumericalFlux3EqnBase & _numerical_flux;

  /// Name of junction component
  const std::string & _junction_name;

  /// Direction material property
  const MaterialProperty<RealVectorValue> & _dir;

  /// Solution vectors for each connection
  std::vector<std::vector<Real>> _solutions;
  /// Flux vector
  std::vector<std::vector<Real>> _fluxes;
  /// Flux Jacobian matrices for each connection
  std::vector<std::vector<DenseMatrix<Real>>> _flux_jacobians;
  /// Degree of freedom indices; first index is connection, second is equation
  std::vector<std::vector<dof_id_type>> _dof_indices;

  /// Element IDs for each connection
  std::vector<unsigned int> _elem_ids;
  /// Local side IDs for each connection
  std::vector<unsigned int> _local_side_ids;

  /// Areas at each connection
  std::vector<Real> _areas;
  /// Directions at each connection
  std::vector<RealVectorValue> _directions;

  /// Connection indices for this thread
  std::vector<unsigned int> _connection_indices;

  /// Pairs of variable names vs. their corresponding equation indices
  static const std::vector<std::pair<std::string, unsigned int>> _varname_eq_index_pairs;

  /// Thread lock
  static Threads::spin_mutex _spin_mutex;

public:
  static InputParameters validParams();
};
