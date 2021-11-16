//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ReferenceResidualProblem.h"
#include "NodeFaceConstraint.h"
#include "MechanicalContactConstraint.h"

/**
 * Class to manage nested solution for augmented Lagrange contact.
 * The AugmentedLagrangianContactProblem manages the nested solution procedure,
 * repeating the solution until convergence has been achieved, checking for convergence, and
 * updating the Lagrangian multipliers.
 */
class AugmentedLagrangianContactProblem : public ReferenceResidualProblem
{
public:
  static InputParameters validParams();

  AugmentedLagrangianContactProblem(const InputParameters & params);
  virtual ~AugmentedLagrangianContactProblem() {}

  virtual void timestepSetup() override;

  virtual MooseNonlinearConvergenceReason
  checkNonlinearConvergence(std::string & msg,
                            const PetscInt it,
                            const Real xnorm,
                            const Real snorm,
                            const Real fnorm,
                            const Real rtol,
                            const Real divtol,
                            const Real stol,
                            const Real abstol,
                            const PetscInt nfuncs,
                            const PetscInt max_funcs,
                            const Real ref_resid,
                            const Real div_threshold) override;

private:
  int _num_lagmul_iterations;
  int _max_lagmul_iters;
};
