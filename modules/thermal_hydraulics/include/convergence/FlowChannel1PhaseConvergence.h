//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiPostprocessorConvergence.h"

/**
 * Assesses convergence of a FlowChannel1Phase component.
 */
class FlowChannel1PhaseConvergence : public MultiPostprocessorConvergence
{
public:
  static InputParameters validParams();

  FlowChannel1PhaseConvergence(const InputParameters & parameters);

protected:
  virtual std::vector<std::tuple<std::string, Real, Real>>
  getDescriptionErrorToleranceTuples() const override;

  virtual unsigned int getMinimumIterations() const override { return 1; }

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
