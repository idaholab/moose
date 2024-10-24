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
#include "PerfGraphInterface.h"

#include "libmesh/equation_systems.h"

// PETSc includes
#include <petsc.h>
#include <petscmat.h>

/**
 * Checks convergence using residual criteria.
 */
class ResidualConvergence : public Convergence
{
public:
  static InputParameters validParams();

  static InputParameters residualConvergenceParams();

  ResidualConvergence(const InputParameters & parameters);

  virtual MooseConvergenceStatus checkConvergence(unsigned int iter) override;

protected:
  /**
   * Check the relative convergence of the nonlinear solution
   * @param fnorm      Norm of the residual vector
   * @param ref_norm   Norm to use for reference value
   * @param rel_tol    Relative tolerance
   * @param abs_tol    Absolute tolerance
   * @return           Bool signifying convergence
   */
  virtual bool checkRelativeConvergence(const PetscInt it,
                                        const Real fnorm,
                                        const Real ref_norm,
                                        const Real rel_tol,
                                        const Real abs_tol,
                                        std::ostringstream & oss);

  /**
   * Performs setup necessary for each call to checkConvergence
   */
  virtual void nonlinearConvergenceSetup(){};

  FEProblemBase & _fe_problem;

  const Real _nl_abs_div_tol;
  const Real _nl_rel_div_tol;
  const Real _div_threshold;
  unsigned int _nl_forced_its;
  const unsigned int _nl_max_pingpong;
  /// Current number of nonlinear ping-pong iterations for the current solve
  unsigned int _nl_current_pingpong;
};
