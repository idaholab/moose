//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef REFERENCERESIDUALPROBLEM_H
#define REFERENCERESIDUALPROBLEM_H

#include "FEProblem.h"

class ReferenceResidualProblem;

template <>
InputParameters validParams<ReferenceResidualProblem>();

/**
 * FEProblemBase derived class to enable convergence checking relative to a user-specified
 * postprocessor
 */
class ReferenceResidualProblem : public FEProblem
{
public:
  ReferenceResidualProblem(const InputParameters & params);
  virtual ~ReferenceResidualProblem();

  virtual void initialSetup() override;
  virtual void timestepSetup() override;
  void updateReferenceResidual();
  virtual MooseNonlinearConvergenceReason
  checkNonlinearConvergence(std::string & msg,
                            const PetscInt it,
                            const Real xnorm,
                            const Real snorm,
                            const Real fnorm,
                            const Real rtol,
                            const Real stol,
                            const Real abstol,
                            const PetscInt nfuncs,
                            const PetscInt max_funcs,
                            const PetscBool force_iteration,
                            const Real ref_resid,
                            const Real div_threshold) override;

  bool checkConvergenceIndividVars(const Real fnorm,
                                   const Real abstol,
                                   const Real rtol,
                                   const Real ref_resid);

  /**
   * Add solution variables to ReferenceResidualProblem.
   * @param sol_vars A set of solution variables that need to be added to ReferenceResidualProblem.
   */
  void addSolutionVariables(std::set<std::string> & sol_vars);

  /**
   * Add reference residual variables to ReferenceResidualProblem.
   * @param ref_vars A set of reference residual variables that need to be added to
   * ReferenceResidualProblem.
   */
  void addReferenceResidualVariables(std::set<std::string> & ref_vars);

  /**
   * Add a set of variables that need to be grouped together.
   * @param group_vars A set of solution variables that need to be grouped.
   */
  void addGroupVariables(std::set<std::string> & group_vars);

protected:
  ///@{
  /// List of solution variable names whose reference residuals will be stored,
  /// and the residual variable names that will store them.
  std::vector<std::string> _soln_var_names;
  std::vector<std::string> _ref_resid_var_names;
  ///@}

  ///@{
  /// List of grouped solution variable names whose reference residuals will be stored,
  /// and the residual variable names that will store them.
  std::vector<std::string> _group_soln_var_names;
  std::vector<std::string> _group_ref_resid_var_names;
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
  /// Local storage for *discrete L2 residual norms* of the variables listed in _ref_resid_var_names.
  std::vector<Real> _ref_resid;
  std::vector<Real> _resid;
  ///@}

  ///@{
  /// Local storage for *discrete L2 residual norms* of the grouped variables listed in _group_ref_resid_var_names.
  std::vector<Real> _group_ref_resid;
  std::vector<Real> _group_resid;
  ///@}

  /// Group number index for each variable
  std::vector<unsigned int> _variable_group_num_index;

  /// Local storage for the scaling factors applied to each of the variables to apply to _ref_resid_vars.
  std::vector<Real> _scaling_factors;

  /// Name of variables that are grouped together to check convergence
  std::vector<std::vector<std::string>> _group_variables;

  /// True if any variables are grouped
  bool _use_group_variables;
};

#endif /* REFERENCERESIDUALPROBLEM_H */
