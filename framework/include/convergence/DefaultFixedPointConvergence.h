//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Convergence.h"

class FixedPointSolve;

/**
 * Default fixed point convergence criteria.
 */
class DefaultFixedPointConvergence : public Convergence
{
public:
  static InputParameters validParams();

  DefaultFixedPointConvergence(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;

  virtual MooseConvergenceStatus checkConvergence(unsigned int iter) override;

protected:
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
  /// Computes and prints the user-specified postprocessor assessing convergence
  void computeCustomConvergencePostprocessor(unsigned int iter);

  /// Whether or not to treat reaching maximum number of fixed point iteration as converged
  const bool _accept_max_it;
  /// Relative tolerance on residual norm
  const Real _fixed_point_rel_tol;
  /// Absolute tolerance on residual norm
  const Real _fixed_point_abs_tol;
  /// Relative tolerance on postprocessor value
  const Real _custom_pp_rel_tol;
  /// Absolute tolerance on postprocessor value
  const Real _custom_pp_abs_tol;

  /// Postprocessor value for user-defined fixed point convergence check
  const PostprocessorValue * const _fixed_point_custom_pp;
  /// Old value of the custom convergence check postprocessor
  Real _pp_old;
  /// Current value of the custom convergence check postprocessor
  Real _pp_new;
  /// Scaling of custom convergence check postprocessor (its initial value)
  Real _pp_scaling;
  /// Convergence history of the custom convergence check postprocessor
  std::ostringstream _pp_history;

  /// Fixed point solve
  FixedPointSolve & _fp_solve;
};

template <typename T>
const T &
DefaultFixedPointConvergence::getSharedExecutionerParam(const std::string & param)
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
