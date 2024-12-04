//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DefaultNonlinearConvergence.h"
#include "ReferenceResidualInterface.h"

// PETSc includes
#include <petsc.h>
#include <petscmat.h>

#include "libmesh/enum_norm_type.h"

/**
 * Uses a reference residual to define relative convergence criteria.
 */
class ReferenceResidualConvergence : public DefaultNonlinearConvergence,
                                     public ReferenceResidualInterface
{
public:
  static InputParameters validParams();

  ReferenceResidualConvergence(const InputParameters & parameters);

  /// Computes the reference residuals for each group
  void updateReferenceResidual();

  virtual void initialSetup() override;

  enum class NormalizationType
  {
    GLOBAL_L2 = 0,
    LOCAL_L2 = 1,
    GLOBAL_LINF = 2,
    LOCAL_LINF = 3
  };

  class ReferenceVectorTagIDKey
  {
    friend class TaggingInterface;
    ReferenceVectorTagIDKey() {}
    ReferenceVectorTagIDKey(const ReferenceVectorTagIDKey &) {}
  };

  /// Returns the tag ID associated with the reference vector tag ID key
  TagID referenceVectorTagID(ReferenceVectorTagIDKey) const { return _reference_vector_tag_id; }

protected:
  virtual void nonlinearConvergenceSetup() override;

  virtual bool checkRelativeConvergence(const unsigned int it,
                                        const Real fnorm,
                                        const Real the_residual,
                                        const Real rtol,
                                        const Real abstol,
                                        std::ostringstream & oss) override;

  /**
   * Check the convergence by comparing the norm of each variable's residual separately against
   * its reference variable's norm. Only consider the solution converged if all
   * variables are converged individually using either a relative or absolute
   * criterion.
   * @param fnorm Function norm (norm of full residual vector)
   * @param abstol Absolute convergence tolerance
   * @param rtol Relative convergence tolerance
   * @param initial_residual_before_preset_bcs Initial norm of full residual vector
   *                                           before applying preset bcs
   * @return true if all variables are converged
   */
  bool checkConvergenceIndividVars(const Real fnorm,
                                   const Real abstol,
                                   const Real rtol,
                                   const Real initial_residual_before_preset_bcs);

  ///@{
  /// List of solution variable names whose reference residuals will be stored,
  /// and the residual variable names that will store them.
  std::vector<NonlinearVariableName> _soln_var_names;
  std::vector<AuxVariableName> _ref_resid_var_names;
  ///@}

  ///@{
  /// List of grouped solution variable names whose reference residuals will be stored,
  /// and the residual variable names that will store them.
  std::vector<NonlinearVariableName> _group_soln_var_names;
  std::vector<AuxVariableName> _group_ref_resid_var_names;
  ///@}

  ///@{
  /// Variable numbers associated with the names in _soln_var_names and _ref_resid_var_names.
  std::vector<unsigned int> _soln_vars;
  std::vector<unsigned int> _ref_resid_vars;
  ///@}

  ///@{
  /// "Acceptable" absolute and relative tolerance multiplier and
  /// acceptable number of iterations.  Used when checking the
  /// convergence of individual variables.
  Real _accept_mult;
  unsigned int _accept_iters;
  ///@}

  ///@{
  /// Local storage for *discrete L2 residual norms* of the grouped variables listed in _group_ref_resid_var_names.
  std::vector<Real> _group_ref_resid;
  std::vector<Real> _group_resid;
  std::vector<Real> _group_output_resid;
  ///@}

  /// Group number index for each variable
  std::vector<unsigned int> _variable_group_num_index;

  /// Local storage for the scaling factors applied to each of the variables to apply to _ref_resid_vars.
  std::vector<Real> _scaling_factors;

  /// The vector storing the reference residual values
  const NumericVector<Number> * _reference_vector;

  /// Variables to use for individual variable convergence checks
  std::vector<NonlinearVariableName> _converge_on;
  /// Flag for each solution variable being in 'converge_on'
  std::vector<bool> _converge_on_var;

  /// Container for convergence treatment when the reference residual is zero
  const enum class ZeroReferenceType { ZERO_TOLERANCE, RELATIVE_TOLERANCE } _zero_ref_type;

  /// Flag to optionally perform normalization of residual by reference residual before or after L2 norm is computed
  bool _local_norm;

  /// Container for normalization type
  libMesh::FEMNormType _norm_type;

  /// The reference vector tag id
  TagID _reference_vector_tag_id;

  bool _initialized;
};
