//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Convergence.h"
#include "FEProblem.h"
#include "PerfGraphInterface.h"

// PETSc includes
#include <petsc.h>
#include <petscmat.h>

#include "libmesh/enum_norm_type.h"

class ReferenceResidualConvergence : public Convergence
{
public:
  static InputParameters validParams();

  ReferenceResidualConvergence(const InputParameters & parameters);

  void setupReferenceResidual();

  void updateReferenceResidual();

  void nonlinearConvergenceSetup();

  bool checkRelativeConvergence(const PetscInt it,
                                const Real fnorm,
                                const Real the_residual,
                                const Real rtol,
                                const Real abstol,
                                std::ostringstream & oss);

  Convergence::MooseAlgebraicConvergence
  checkAlgebraicConvergence(int it, Real xnorm, Real snorm, Real fnorm) override;

  /**
   * Check the convergence by comparing the norm of each variable separately against
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

  /**
   * Add a set of variables that need to be grouped together. For use in
   * actions that create variables. This is templated for backwards compatibility to allow passing
   * in std::string or NonlinearVariableName.
   * @param group_vars A set of solution variables that need to be grouped.
   */
  template <typename T>
  void addGroupVariables(const std::set<T> & group_vars);

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

  TagID referenceVectorTagID(ReferenceVectorTagIDKey) const { return _reference_vector_tag_id; }

protected:
  FEProblemBase & _fe_problem;

  PerfID _perf_nonlinear;

  // Variables for the convergence criteria
  Real _atol; // absolute convergence tolerance
  Real _rtol; // relative convergence tolerance
  Real _stol; // convergence (step) tolerance in terms of the norm of the change in the
              // solution between steps

  Real _div_threshold = std::numeric_limits<Real>::max();
  /// the absolute non linear divergence tolerance
  Real _nl_abs_div_tol = -1;
  Real _divtol; // relative divergence tolerance

  Real _nl_rel_tol;
  Real _nl_abs_tol;
  Real _nl_rel_step_tol;
  Real _nl_abs_step_tol;

  int _nl_forced_its = 0; // the number of forced nonlinear iterations
  PetscInt _nfuncs = 0;

  unsigned int _nl_max_its;
  unsigned int _nl_max_funcs;

  PetscInt _maxit; // maximum number of iterations
  PetscInt _maxf;  // maximum number of function evaluations

  // Linear solver convergence criteria
  Real _l_tol;
  Real _l_abs_tol;
  unsigned int _l_max_its;

  /// maximum number of ping-pong iterations
  unsigned int _n_nl_pingpong = 0;
  unsigned int _n_max_nl_pingpong = std::numeric_limits<unsigned int>::max();

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
  /// Variable numbers assoicated with the names in _soln_var_names and _ref_resid_var_names.
  std::vector<unsigned int> _soln_vars;
  std::vector<unsigned int> _ref_resid_vars;
  ///@}

  ///@{
  /// "Acceptable" absolute and relative tolerance multiplier and
  /// acceptable number of iterations.  Used when checking the
  /// convergence of individual variables.
  Real _accept_mult;
  int _accept_iters;
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

  /// Name of variables that are grouped together to check convergence
  std::vector<std::vector<NonlinearVariableName>> _group_variables;

  /// True if any variables are grouped
  bool _use_group_variables;

  /// Container for convergence treatment when the reference residual is zero
  const enum class ZeroReferenceType { ZERO_TOLERANCE, RELATIVE_TOLERANCE } _zero_ref_type;

  /// The vector storing the reference residual values
  const NumericVector<Number> * _reference_vector;

  std::vector<NonlinearVariableName> _converge_on;
  std::vector<bool> _converge_on_var;

  /// Flag to optionally perform normalization of residual by reference residual before or after L2 norm is computed
  bool _local_norm;

  /// Container for normalization type
  FEMNormType _norm_type;

  /// The reference vector tag id
  TagID _reference_vector_tag_id;

  bool _initialized;
};

template <typename T>
void
ReferenceResidualConvergence::addGroupVariables(const std::set<T> & group_vars)
{
  _group_variables.push_back(
      std::vector<NonlinearVariableName>(group_vars.begin(), group_vars.end()));
  _use_group_variables = true;
}
