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

/**
 * Assesses convergence of a FlowChannel1Phase component.
 */
class FlowChannel1PhaseConvergence : public Convergence
{
public:
  static InputParameters validParams();

  FlowChannel1PhaseConvergence(const InputParameters & parameters);

  virtual MooseConvergenceStatus checkConvergence(unsigned int iter) override;

protected:
  /// Generates a colored line for a tolerance comparison
  std::string comparisonLine(const std::string & description, Real err, Real tol) const;

  // step errors
  const PostprocessorValue & _p_rel_step;
  const PostprocessorValue & _T_rel_step;
  const PostprocessorValue & _vel_rel_step;

  // residual errors
  const PostprocessorValue & _mass_res;
  const PostprocessorValue & _momentum_res;
  const PostprocessorValue & _energy_res;

  // step tolerances
  const Real _p_rel_step_tol;
  const Real _T_rel_step_tol;
  const Real _vel_rel_step_tol;

  // residual tolerances
  const Real _mass_res_tol;
  const Real _momentum_res_tol;
  const Real _energy_res_tol;
};
