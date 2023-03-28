//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MappingBase.h"
#include "ParallelSolutionStorage.h"
#include <slepcsvd.h>
#include "libmesh/parallel_object.h"
#include "libmesh/petsc_vector.h"

class PODMapping : public MappingBase
{
public:
  static InputParameters validParams();
  PODMapping(const InputParameters & parameters);

  ~PODMapping();

  virtual void buildMapping(const VariableName & vname) override;

  ///@{
  /**
   * Methods used for mapping full-order solutions for a given variable
   * onto a latent space
   */
  void map(const VariableName & vname,
           const DenseVector<Real> & full_order_vector,
           std::vector<Real> & reduced_order_vector) const override;

  void map(const VariableName & vname,
           const unsigned int global_sample_i,
           std::vector<Real> & reduced_order_vector) const override;
  ///@}

  /**
   * Method used for mapping reduced-order solutions for a given variable
   * onto the full-order space
   */
  void inverse_map(const VariableName & vname,
                   const std::vector<Real> & reduced_order_vector,
                   DenseVector<Real> & full_order_vector) const override;

  /**
   * Get the `base_i`-th basis function for a given variable
   * @param vname The name of the variable
   * @param base_i The index of the basis function
   * @return const DenseVector<Real>&
   */
  const DenseVector<Real> & basis(const VariableName & vname, const unsigned int base_i);

protected:
  /**
   * Determine the number of basis functions needed for a given variable based on the information
   * on the eigenvalues.
   * @param vname The name of the variable
   * @param converged_evs Vector of converged eigenvalues
   */
  unsigned int determineNumberOfModes(const VariableName & vname,
                                      const std::vector<Real> & converged_evs);

  /// The number of modes requested by the user
  const std::vector<unsigned int> & _num_modes;

  /// The energy thresholds for truncation of the number of modes, defined by the user
  const std::vector<Real> & _energy_threshold;

  /// Restartable container holding the basis functions for each variable
  std::map<VariableName, std::vector<DenseVector<Real>>> & _basis_functions;

  /// Restartable container holding the singular values
  std::map<VariableName, std::vector<Real>> & _singular_values;

  /// Variable holding additional petsc options for the singular value solve
  const std::string & _extra_slepc_options;

private:
  /// Link to the parallel storage which holds the solution fields that are used for the SVD
  ParallelSolutionStorage * _parallel_storage;

  /// Storage for SLEPC's SVD objects for each variable.
  std::map<VariableName, SVD> _svds;

  /// Bool to decide if we already have the SVD or not to make sure it is
  /// not computed multiple times unless the user requests it
  std::map<VariableName, bool> _computed_svd;
};
