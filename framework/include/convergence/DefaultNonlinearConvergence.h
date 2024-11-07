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
#include "MooseApp.h"
#include "Executioner.h"

/**
 * Default convergence criteria for FEProblem
 */
class DefaultNonlinearConvergence : public Convergence
{
public:
  static InputParameters validParams();

  static InputParameters residualConvergenceParams();

  DefaultNonlinearConvergence(const InputParameters & parameters);

  virtual void initialSetup() override;

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

  /**
   * This method is to be used for parameters that are shared with the executioner.
   *
   * If the parameter is set by the user in the executioner, get that value;
   * otherwise, get this object's value.
   * If the parameter is set by the user in both this object and the executioner,
   * add it to a list and report an error later.
   */
  template <typename T>
  const T & getSharedExecutionerParam(const std::string & name);

  /**
   * Throws an error if any of the parameters shared with the executioner have
   * been set by the user in both places.
   *
   * This should be called after all calls to \c getSharedExecutionerParam.
   * No error is thrown if the Convergence object was added as a default, since
   * in that case, any parameters set by the user in the executioner will also
   * be considered set by the user in the Convergence.
   */
  void checkDuplicateSetSharedExecutionerParams() const;

private:
  /// True if this object was added as a default instead of by the user
  const bool _added_as_default;

  /// List of shared executioner parameters that have been set by the user in both places
  std::vector<std::string> _duplicate_shared_executioner_params;

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

template <typename T>
const T &
DefaultNonlinearConvergence::getSharedExecutionerParam(const std::string & param)
{
  const auto * executioner = getMooseApp().getExecutioner();
  if (executioner->isParamSetByUser(param))
  {
    if (isParamSetByUser(param))
      _duplicate_shared_executioner_params.push_back(param);
    return executioner->getParam<T>(param);
  }
  else
    return getParam<T>(param);
}
