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

protected:
  ///@{
  /// List of solution variable names whose reference residuals will be stored,
  /// and the residual variable names that will store them.
  std::vector<std::string> _solnVarNames;
  std::vector<std::string> _refResidVarNames;
  ///@}

  ///@{
  /// Variable numbers assoicated with the names in _solnVarNames and _refResidVarNames.
  std::vector<unsigned int> _solnVars;
  std::vector<unsigned int> _refResidVars;
  ///@}

  ///@{
  /// "Acceptable" absolute and relative tolerance multiplier and
  /// acceptable number of iterations.  Used when checking the
  /// convergence of individual variables.
  Real _accept_mult;
  int _accept_iters;
  ///@}

  ///@{
  /// Local storage for *discrete L2 residual norms* of the variables listed in _refResidVarNames.
  std::vector<Real> _refResid;
  std::vector<Real> _resid;
  ///@}

  /// Local storage for the scaling factors applied to each of the variables to apply to _refResidVars.
  std::vector<Real> _scaling_factors;
};

#endif /* REFERENCERESIDUALPROBLEM_H */
