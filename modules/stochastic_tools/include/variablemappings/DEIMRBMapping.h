//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ReporterInterface.h"
#include "VariableMappingBase.h"
#include "ParallelSolutionStorage.h"
#include "UserObjectInterface.h"

#include <slepcsvd.h>
#include "POD.h"

/**
 * Class for integrating Discrete Empirical Interpolation Method (DEIM) with
 * Proper Orthogonal Decomposition (POD)-based Reduced Basis (RB) mapping.
 * This class specializes in mapping between full-order model spaces and
 * reduced-order model spaces, leveraging POD for dimensionality
 * reduction and DEIM for efficient nonlinear approximation. This class assumes
 * that the parallel storage has a solution, residual, and jacobian variable name.
 */
class DEIMRBMapping : public VariableMappingBase,
                      public UserObjectInterface,
                      public ReporterInterface
{
public:
  static InputParameters validParams();
  DEIMRBMapping(const InputParameters & parameters);

  virtual void buildMapping(const VariableName & /*vname*/) override
  {
    mooseError("Need to build all mappings concurrently with DEIMRBMapping.");
  };
  /**
   * Builds the mappings of all the components in the DEIM RB system.
   */
  virtual void buildAllMappings() override;

  /**
   * Method used for mapping full-order solutions for a given variable
   * onto a latent space
   * @param vname The name of the variable
   * @param full_order_vector Serialized vector of the solution field for the given variable
   * @param reduced_order_vector Storage space for the coordinates in the latent space
   */
  void map(const VariableName & vname,
           const DenseVector<Real> & full_order_vector,
           std::vector<Real> & reduced_order_vector) const override;

  /**
   * Method used for mapping full-order solutions for a given variable
   * onto a latent space
   * @param vname The name of the variable
   * @param global_sample_i The global index of the sample whose solution should be mapped
   *                        into the latent space
   * @param reduced_order_vector Storage space for the coordinates in the latent space
   */
  void map(const VariableName & vname,
           const unsigned int global_sample_i,
           std::vector<Real> & reduced_order_vector) const override;

  /**
   * Method used for mapping reduced-order solutions for a given variable
   * onto the full-order space
   * @param vname The name of the variable
   * @param reduced_order_vector The coordinates in the latent space
   * @param full_order_vector Storage for the reconstructed solution for the given variable
   */
  void inverse_map(const VariableName & vname,
                   const std::vector<Real> & reduced_order_vector,
                   DenseVector<Real> & full_order_vector) const override;

  /**
   * Computes the reduced jacobian using the selection values. This function
   * assumes that the jacobian values are in the same order as the selection indicies
   * @param red_jac_values vector of the jacobian values
   * @return RealDenseMatrix Reduced jacobian matrix
   */
  RealDenseMatrix compute_reduced_jac(const std::vector<Real> & red_jac_values);

  /**
   * Computes the reduced residual vector using the given residual values. This function
   * assumes that the residual values are provided in a specific order corresponding to
   * the problem's requirements or the arrangement of the system equations.
   *
   * @param red_res_values A vector of residual values used to compute the reduced residual.
   * @return DenseVector<Real> The reduced residual vector.
   */
  DenseVector<Real> compute_reduced_res(const std::vector<Real> & red_res_values);

  /**
   * Gets the selection indices for the residual.
   * @return A reference to the vector of DOF indices for the residual.
   */
  const std::vector<dof_id_type> getResidualSelectionIndices() const
  {
    return _residual_selection_inds;
  }

  /**
   * Gets the reduced DOFs needed during the online phase.
   * @return A reference to the set of reduced DOFs.
   */
  const std::vector<std::pair<dof_id_type, dof_id_type>> getJacobianSelectionIndices() const
  {
    return _jac_matrix_selection_inds;
  }

  dof_id_type getReducedSize()
  {
    checkIfReadyToUse("solution");
    return _sol_basis.size();
  }

protected:
  /// The number of modes which need to be computed
  const std::vector<dof_id_type> _num_modes;

  /// The energy thresholds for truncation of the number of modes, defined by the user
  const std::vector<Real> & _energy_threshold;

  /// Restartable container holding the basis functions for the solution
  std::vector<DenseVector<Real>> & _sol_basis;

  /// Restartable vector holding the selection indices for the residual
  std::vector<dof_id_type> & _residual_selection_inds;

  /// Restartable vector holding the selection indices for the jacobian
  std::vector<dof_id_type> & _jacobian_selection_inds;
  /// Restartable vector holding the reduced dofs that are need during online phase
  std::vector<std::pair<dof_id_type, dof_id_type>> & _jac_matrix_selection_inds;

  /// Restartable vector holding the residual's reduced basis
  std::vector<DenseVector<Real>> & _reduced_residual;
  /// Restartable vector holding the reduced jacobian matrix basis
  std::vector<DenseMatrix<Real>> & _reduced_jacobian;

  /// Restartable matrix holding the residual's reduced basis
  DenseMatrix<Real> & _residual_selection_matrix;
  /// Restartable matrix holding the jacobian's reduced basis
  DenseMatrix<Real> & _jacobian_selection_matrix;

  /// Residual basis needed for computing but should not be stored
  std::vector<DenseVector<Real>> _res_basis;
  /// Jacobian basis needed for computing but should not be stored
  std::vector<DenseVector<Real>> _jac_basis;

private:
  /**
   * Build the basis for each component using POD
   */
  virtual void buildComponentBases();
  /**
   * Construct the jacobian matrix basis from the row-concatenated vectors
   */
  virtual void constructReducedBasis();
  /**
   * Get the selection indicies for the residual and jacobian
   */
  virtual void computeDEIMSelectionIndices();
  /**
   * Create a selection matrix from the selection indicies and basis
   */
  virtual void storeSelectionMatrices();

  std::vector<DenseVector<Real>> computeBasis(const VariableName & vname);

  void computeSparseMatrixConversion();
  /// Link to the parallel storage which holds the solution fields that are used for the SVD
  const ParallelSolutionStorage * const _parallel_storage;

  /// The POD object which can be used to compute the basis functions/vectors
  const StochasticTools::POD _pod;

  /// From the column concatenated indices, extract the unique reduced dofs
  std::vector<std::pair<dof_id_type, dof_id_type>>
  convertMatrixSelectionInds(dof_id_type num_dofs, const std::vector<dof_id_type> & indices);

  const std::vector<std::pair<dof_id_type, dof_id_type>> * _jacobian_indices;
};
