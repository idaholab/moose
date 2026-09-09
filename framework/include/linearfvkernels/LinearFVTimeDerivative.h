//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  /// Whether to assemble d(cu)/dt rather than c du/dt
  const bool _conservative_form;

private:
  /// Values of the material property multiplier, one per history slot of the time integrator.
  /// The integrator pairs entry i with the solution i+1 steps back, so in the conservative form
  /// these are the multiplier at those same times, and in the non-conservative form they are all
  /// the current multiplier.
  std::vector<Real> _factor_history;

  /// State args used to fetch the entries of _factor_history above.
  std::vector<Moose::StateArg> _state_args;

  /// The multiplier that scales the new solution, always the current one in either form
  const Moose::StateArg _current_state;
};
