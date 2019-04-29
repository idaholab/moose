#pragma once

#include "FlowJunctionUserObject.h"

class VolumeJunctionBaseUserObject;

template <>
InputParameters validParams<VolumeJunctionBaseUserObject>();

/**
 * Base class for computing and caching flux and residual vectors for a volume junction
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the junction, and
 * \li fluxes between the flow channels and the junction.
 */
class VolumeJunctionBaseUserObject : public FlowJunctionUserObject
{
public:
  /**
   * Constructor.
   *
   * @param[in] params                  Input parameters
   * @param[in] flow_variable_names     Vector of coupled flow variable names,
   *                                    indexed by equation index
   * @param[in] scalar_variable_names   Vector of coupled scalar variable names,
   *                                    indexed by equation index
   */
  VolumeJunctionBaseUserObject(const InputParameters & params,
                               const std::vector<std::string> & flow_variable_names,
                               const std::vector<std::string> & scalar_variable_names);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

  /**
   * Returns the residual vector for the scalar variables
   */
  const std::vector<Real> & getResidual() const;

  /**
   * Returns the flux vector associated with a connection
   *
   * @param[in] connection_index   Connection index
   */
  const std::vector<Real> & getFlux(const unsigned int & connection_index) const;

  /**
   * Returns the Jacobian matrix of a connection's flux vector w.r.t. the scalar variables
   *
   * @param[in] connection_index   Connection index
   */
  const DenseMatrix<Real> &
  getFluxJacobianScalarVariables(const unsigned int & connection_index) const;

  /**
   * Returns the Jacobian matrix of a connection's flux vector w.r.t. its flow channel variables
   *
   * @param[in] connection_index   Connection index
   */
  const DenseMatrix<Real> &
  getFluxJacobianFlowChannelVariables(const unsigned int & connection_index) const;

  /**
   * Gets the Jacobian data for the scalar equations
   *
   * @param[in] equation_index   Local scalar equation index
   * @param[out] jacobian_block  Jacobian matrix in which to store entries
   * @param[out] dofs_i          Vector in which to store corresponding DoF rows
   * @param[out] dofs_j          Vector in which to store corresponding DoF columns
   */
  void getScalarEquationJacobianData(const unsigned int & equation_index,
                                     DenseMatrix<Real> & jacobian_block,
                                     std::vector<dof_id_type> & dofs_i,
                                     std::vector<dof_id_type> & dofs_j) const;

protected:
  /**
   * Computes and stores the fluxes, the scalar residuals, and their Jacobians.
   *
   * @param[in] c   Connection index
   */
  virtual void computeFluxesAndResiduals(const unsigned int & c) = 0;

  /**
   * Computes and stores the Jacobian entries of the scalar equations w.r.t.
   * flow channel dofs.
   *
   * The main input to this function is the Jacobian matrix of the scalar equations
   * w.r.t. to the flow channel solution \e functions. This function then applies
   * the chain rule of the derivative of the solution \e functions w.r.t. the
   * solution \e dofs. Thus, the basis functions (\c phi) are applied with this
   * function.
   *
   * @param[in] jac   Jacobian matrix of the scalar equations w.r.t. flow solution functions
   * @param[in] c     Connection index
   */
  void computeScalarJacobianWRTFlowDofs(const DenseMatrix<Real> & jac, const unsigned int & c);

  /// Volume of the junction
  const Real & _volume;

  /// Vector of coupled variable names for each flow variable
  const std::vector<std::string> _flow_variable_names;
  /// Vector of coupled variable names for each scalar variable
  const std::vector<std::string> _scalar_variable_names;

  /// Number of flow channel flux components
  const unsigned int _n_flux_eq;
  /// Number of scalar residual components
  const unsigned int _n_scalar_eq;

  /// Names of numerical flux user objects for each connected flow channel
  const std::vector<UserObjectName> & _numerical_flux_names;

  /// Connection indices for this thread
  std::vector<unsigned int> _connection_indices;

  /// Degrees of freedom for scalar variables
  std::vector<dof_id_type> _scalar_dofs;
  /// Degrees of freedom for flow channel variables, for each connection
  std::vector<std::vector<dof_id_type>> _flow_channel_dofs;

  /// Cached flux vector for each connection
  std::vector<std::vector<Real>> _flux;
  /// Cached flux Jacobian matrices w.r.t. scalar variables, for each connection
  std::vector<DenseMatrix<Real>> _flux_jacobian_scalar_vars;
  /// Cached flux Jacobian matrices w.r.t. flow channel variables, for each connection
  std::vector<DenseMatrix<Real>> _flux_jacobian_flow_channel_vars;

  /// Cached scalar residual vector
  std::vector<Real> _residual;
  /// Cached scalar residual Jacobian entries w.r.t. scalar variables for each scalar variable
  std::vector<DenseMatrix<Real>> _residual_jacobian_scalar_vars;
  /// Cached scalar residual Jacobian matrices w.r.t. flow channel variables,
  /// for each connection (first index) and each scalar variable (second index)
  std::vector<std::vector<DenseMatrix<Real>>> _residual_jacobian_flow_channel_vars;
};
