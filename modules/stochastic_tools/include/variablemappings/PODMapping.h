//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VariableMappingBase.h"
#include "ParallelSolutionStorage.h"
#include "UserObjectInterface.h"

#include <slepcsvd.h>
#include "libmesh/parallel_object.h"
#include "libmesh/petsc_vector.h"

/**
 * Class which provides a Proper Orthogonal Decomposition (POD)-based mapping between
 * full-order and reduced-order spaces.
 */
class PODMapping : public VariableMappingBase, public UserObjectInterface
{
public:
  static InputParameters validParams();
  PODMapping(const InputParameters & parameters);

  virtual ~PODMapping();

  virtual void buildMapping(const VariableName & vname) override;

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
   * Return all of the left basis functions for a given variable
   * @param vname The name of the variable.
   */
  const std::vector<DenseVector<Real>> & leftBasis(const VariableName & vname);

  /**
   * Return all of the right basis functions for a given variable
   * @param vname The name of the variable.
   */
  const std::vector<DenseVector<Real>> & rightBasis(const VariableName & vname);

  /**
   * Return all of the singular values for a given variable
   * @param vname The name of the variable.
   */
  const std::vector<Real> & singularValues(const VariableName & vname);

  /**
   * Get the `base_i`-th left basis function for a given variable
   * @param vname The name of the variable
   * @param base_i The index of the basis function
   */
  const DenseVector<Real> & leftBasisFunction(const VariableName & vname,
                                              const unsigned int base_i);

  /**
   * Get the `base_i`-th right basis function for a given variable
   * @param vname The name of the variable
   * @param base_i The index of the basis function
   */
  const DenseVector<Real> & rightBasisFunction(const VariableName & vname,
                                               const unsigned int base_i);

protected:
  /**
   * Determine the number of basis functions needed for a given variable based on the information
   * on the eigenvalues.
   * @param vname The name of the variable
   * @param converged_evs Vector of converged eigenvalues
   */
  dof_id_type determineNumberOfModes(const VariableName & vname,
                                     const std::vector<Real> & converged_evs);

  /// The number of modes which need to be computed
  const std::vector<dof_id_type> _num_modes;

  /// The energy thresholds for truncation of the number of modes, defined by the user
  const std::vector<Real> & _energy_threshold;

  /// Restartable container holding the basis functions for each variable
  std::map<VariableName, std::vector<DenseVector<Real>>> & _left_basis_functions;

  /// Restartable container holding the basis functions for each variable
  std::map<VariableName, std::vector<DenseVector<Real>>> & _right_basis_functions;

  /// Restartable container holding the singular values
  std::map<VariableName, std::vector<Real>> & _singular_values;

  /// Variable holding additional petsc options for the singular value solve
  const std::string & _extra_slepc_options;

private:
  /// Link to the parallel storage which holds the solution fields that are used for the SVD
  const ParallelSolutionStorage * const _parallel_storage;

#if !PETSC_VERSION_LESS_THAN(3, 14, 0)
  /// Storage for SLEPC's SVD objects for each variable.
  std::map<VariableName, SVD> _svds;
#endif
};
