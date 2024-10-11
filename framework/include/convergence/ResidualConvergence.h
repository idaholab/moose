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
   * @param rtol       Relative tolerance
   * @param abstol     Absolute tolerance
   * @return           Bool signifying convergence
   */
  virtual bool checkRelativeConvergence(const PetscInt it,
                                        const Real fnorm,
                                        const Real ref_norm,
                                        const Real rtol,
                                        const Real abstol,
                                        std::ostringstream & oss);

  /**
   * Performs setup necessary for each call to checkConvergence
   */
  virtual void nonlinearConvergenceSetup(){};

  /**
   * Gets a parameter if the user supplies and else takes from EquationSystems object
   *
   * @param[in] conv_param   Parameter name in this convergence object
   * @param[in] es_param     Parameter name in the EquationSystems object
   * @param[in] es           EquationSystems object
   */
  template <typename T>
  T getSharedESParam(const std::string & conv_param,
                     const std::string & es_param,
                     EquationSystems & es) const;

  FEProblemBase & _fe_problem;

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

  PetscErrorCode ierr;
  PetscInt it_petsc;
  PetscReal xnorm_petsc;
  PetscReal fnorm_petsc;
  PetscReal snorm_petsc;
};

template <typename T>
T
ResidualConvergence::getSharedESParam(const std::string & conv_param,
                                      const std::string & es_param,
                                      EquationSystems & es) const
{
  if (isParamSetByUser(conv_param))
  {
    es.parameters.set<T>(es_param) = getParam<T>(conv_param);
    return getParam<T>(conv_param);
  }
  else
    return es.parameters.get<T>(es_param);
}
