//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InversionControlBase.h"

/**
 * Control that adjusts a scalar parameter (held in a postprocessor) to match a sub-app output to a
 * target function, using a finite-difference Newton update. Because a Control cannot safely drive
 * its own perturbed sub-app solve (the fixed-point executioner owns the MultiApp's single backup
 * slot), the local derivative is formed over two consecutive fixed-point iterations: a "base" solve
 * at p and a "perturbed" solve at p + parameter_delta. Convergence, the outer loop, and time-step
 * cutting are owned by the executioner and the Convergence system.
 */
class NewtonInversionControl : public InversionControlBase
{
public:
  static InputParameters validParams();

  NewtonInversionControl(const InputParameters & parameters);

protected:
  virtual IterationUpdate
  computeUpdate(unsigned int it, Real p_used, Real y, Real y_target) override;

private:
  /// Finite-difference perturbation applied on each base iteration to form df/dp
  const Real _parameter_delta;
  /// Residual written on perturbed iterations so convergence is only declared on base iterations
  const Real _nonconverged_residual;
  /// Parameter value of the current base solve (re-seeded each sweep; not restartable)
  Real _p_base;
  /// Output value of the current base solve (re-seeded each sweep; not restartable)
  Real _y_base;
};
