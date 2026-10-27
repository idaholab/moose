//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  class ReferenceVectorTagIDKey
  {
    friend class TaggingInterface;
    ReferenceVectorTagIDKey() {}
    ReferenceVectorTagIDKey(const ReferenceVectorTagIDKey &) {}
  };

  /// Returns the tag ID associated with the reference vector tag ID key
  TagID referenceVectorTagID(ReferenceVectorTagIDKey) const { return _reference_vector_tag_id; }

protected:
  virtual NonlinearSystemBase & nonlinearSystem() override;

  virtual void nonlinearConvergenceSetup() override;

  virtual bool checkResidualConvergence(const unsigned int n_iter,
                                        const Real fnorm,
                                        const Real ref_norm,
                                        const Real rel_tol,
                                        const Real abs_tol,
                                        std::ostringstream & oss) override;

  /**
   * Check the convergence by comparing the norm of each variable's residual separately against
   * its reference variable's norm. Only consider the solution converged if all
   * variables are converged individually using either a relative or absolute
   * criterion.
   * @param fnorm Function norm (norm of full residual vector)
   * @param abs_tol Absolute convergence tolerance
   * @param rel_tol Relative convergence tolerance
   * @param initial_residual_before_preset_bcs Initial norm of full residual vector
   *                                           before applying preset bcs
   * @return true if all variables are converged
   */
  bool checkConvergenceIndividVars(const Real fnorm,
                                   const Real abs_tol,
                                   const Real rel_tol,
                                   const Real initial_residual_before_preset_bcs);

  /// Enum holding the normalization type
  const MooseEnum _norm_type_enum;

  ///@{
  /// List of solution variable names whose reference residuals will be stored,
  /// and the residual variable names that will store them.
  std::vector<NonlinearVariableName> _soln_var_names;
  std::vector<AuxVariableName> _ref_resid_var_names;
  ///@}

  ///@{
  /// List of grouped solution variable names whose reference residuals will be stored
  std::vector<NonlinearVariableName> _group_names;
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
  const Real _accept_mult;
  const unsigned int _accept_iters;
  ///@}

  /// Nonlinear system to which this convergence object applies.
  const unsigned int _nl_sys_num;

  ///@{
  /// Local storage for *discrete L2 residual norms* of the grouped variables.
  std::vector<Real> _group_ref_resid;
  std::vector<Real> _group_resid;
  ///@}

  /// Vector of bools to signify if variable is in a group.
  std::vector<bool> _is_var_grouped;

  /// Group number index for each variable
  std::vector<unsigned int> _group_index;

  /// Local storage for the scaling factors applied to each of the variables to apply to _ref_resid_vars.
  std::vector<Real> _scaling_factors;

  /// The optional vector storing the reference residual values
  const NumericVector<Number> * _residual_vector;

  /// The vector storing the reference residual values
  const NumericVector<Number> * _reference_vector;

  /// Flag for each solution variable or group being in 'converge_on'
  std::vector<bool> _converge_on_var;
  std::vector<bool> _converge_on_group;

  /// Container for convergence treatment when the reference residual is zero
  const enum class ZeroReferenceType { ZERO_TOLERANCE, RELATIVE_TOLERANCE } _zero_ref_type;

  /// Bool to unscale the residual before convergence checks and screen output
  const bool _unscale_the_residual;

  /// Flag to optionally perform normalization of residual by reference residual before or after L2 norm is computed
  bool _local_norm;

  /// Container for normalization type
  libMesh::FEMNormType _norm_type;

  /// The reference vector tag id
  TagID _reference_vector_tag_id;
};
