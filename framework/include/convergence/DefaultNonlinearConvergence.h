//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DefaultConvergenceBase.h"
#include "MooseApp.h"
#include "Executioner.h"

/**
 * Default nonlinear convergence criteria for FEProblem
 */
class DefaultNonlinearConvergence : public DefaultConvergenceBase
{
public:
  static InputParameters validParams();
  static InputParameters residualConvergenceParams();

  DefaultNonlinearConvergence(const InputParameters & parameters);

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
  virtual bool checkRelativeConvergence(const unsigned int it,
                                        const Real fnorm,
                                        const Real ref_norm,
                                        const Real rel_tol,
                                        const Real abs_tol,
                                        std::ostringstream & oss);

  /**
   * Performs setup necessary for each call to checkConvergence
   */
  virtual void nonlinearConvergenceSetup() {}

protected:
  FEProblemBase & _fe_problem;
  /// Nonlinear absolute divergence tolerance
  const Real _nl_abs_div_tol;
  /// Nonlinear relative divergence tolerance
  const Real _nl_rel_div_tol;
  /// Divergence threshold value
  const Real _div_threshold;
  /// Number of iterations to force
  unsigned int _nl_forced_its;
  /// Maximum number of nonlinear ping-pong iterations for a solve
  const unsigned int _nl_max_pingpong;
  /// Current number of nonlinear ping-pong iterations for the current solve
  unsigned int _nl_current_pingpong;
};
