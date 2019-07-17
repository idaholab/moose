#pragma once

#include "FlowJunctionUserObject.h"
#include "DerivativeMaterialInterfaceTHM.h"

class GateValve1PhaseUserObject;
class NumericalFlux3EqnBase;

template <>
InputParameters validParams<GateValve1PhaseUserObject>();

/**
 * Gate valve user object for 1-phase flow
 */
class GateValve1PhaseUserObject : public DerivativeMaterialInterfaceTHM<FlowJunctionUserObject>
{
public:
  GateValve1PhaseUserObject(const InputParameters & params);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

  /**
   * Gets the flux vector for a connection
   *
   * @param[in] connection_index   Index for the connection
   */
  const std::vector<Real> & getFlux(const unsigned int & connection_index) const;

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
  /// Fraction of possible flow area that is open
  const Real & _f_open;
  /// Minimum open area fraction
  const Real & _f_open_min;

  /// Cross-sectional area of connected flow channels
  const VariableValue & _A;
  /// rho*A of the connected flow channels
  const VariableValue & _rhoA;
  /// rho*u*A of the connected flow channels
  const VariableValue & _rhouA;
  /// rho*E*A of the connected flow channels
  const VariableValue & _rhoEA;

  /// Flow channel rho*A coupled variable index
  const unsigned int _rhoA_jvar;
  /// Flow channel rho*u*A coupled variable index
  const unsigned int _rhouA_jvar;
  /// Flow channel rho*E*A coupled variable index
  const unsigned int _rhoEA_jvar;

  /// Pressure material property
  const MaterialProperty<Real> & _p;
  const MaterialProperty<Real> & _dp_drhoA;
  const MaterialProperty<Real> & _dp_drhouA;
  const MaterialProperty<Real> & _dp_drhoEA;

  /// Numerical flux user object
  const NumericalFlux3EqnBase & _numerical_flux;

  /// Name of the associated component
  const std::string & _component_name;

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

  /// Stored pressure for each connection
  std::vector<Real> _stored_p;
  std::vector<Real> _stored_dp_drhoA;
  std::vector<Real> _stored_dp_drhouA;
  std::vector<Real> _stored_dp_drhoEA;

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
};
