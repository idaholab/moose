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

class TransientBase;
class AuxiliarySystem;

/**
 * Default steady-state convergence criteria.
 */
class DefaultSteadyStateConvergence : public DefaultConvergenceBase
{
public:
  static InputParameters validParams();

  DefaultSteadyStateConvergence(const InputParameters & parameters);

  virtual void checkIterationType(IterationType it_type) const override;

  virtual MooseConvergenceStatus checkConvergence(unsigned int iter) override;

protected:
  /// Steady-state tolerance for the solution variables or aux variables
  const Real _steady_state_tolerance;
  /// Whether to use the auxiliary system solution to determine steady-state
  const bool _check_aux;
  /// Whether to normalize solution norm by time step size
  const bool _normalize_norm_by_dt;

  /// FE problem
  FEProblemBase & _fe_problem;
  /// Transient executioner
  TransientBase * const _transient_executioner;
  /// Aux system
  AuxiliarySystem & _aux_system;
};
