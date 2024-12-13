//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVElementalKernel.h"
#include "TimeIntegrator.h"

/**
 * Kernel that adds contributions from a time derivative term to a linear system
 * populated using the finite volume method.
 */
class LinearFVTimeDerivative : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVTimeDerivative(const InputParameters & params);

  virtual Real computeMatrixContribution() override;

  virtual Real computeRightHandSideContribution() override;

  virtual void setCurrentElemInfo(const ElemInfo * elem_info) override;

protected:
  /// The functor for the material property multipler
  const Moose::Functor<Real> & _factor;

  /// The time integrator to use in this kernel, will provide information
  /// on how many states are required in the history.
  const TimeIntegrator & _time_integrator;

private:
  /// Current and older values of the material property multiplier.
  std::vector<Real> _factor_history;

  /// State args, the args which will help us fetch the different states of
  /// the material property multiplier. 0th is the current, 1st is old
  /// 2nd is the older. Might not need all, depends on the time integrator.
  std::vector<Moose::StateArg> _state_args;
};
