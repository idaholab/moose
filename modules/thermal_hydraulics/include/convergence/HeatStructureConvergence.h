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
 * Assesses convergence of a HeatStructure component.
 */
class HeatStructureConvergence : public MultiPostprocessorConvergence
{
public:
  static InputParameters validParams();

  HeatStructureConvergence(const InputParameters & parameters);

protected:
  virtual std::vector<std::tuple<std::string, Real, Real>>
  getDescriptionErrorToleranceTuples() const override;

  virtual unsigned int getMinimumIterations() const override { return 1; }

  /// Heat structure blocks
  const std::vector<SubdomainName> & _blocks;

  /// Temperature relative step for each block
  std::vector<const PostprocessorValue *> _T_rel_step;
  /// Residual error for each block
  std::vector<const PostprocessorValue *> _res;

  /// Temperature relative step tolerance
  const Real _T_rel_step_tol;
  /// Residual tolerance
  const Real _res_tol;

  /// Number of blocks
  const unsigned int _n_blocks;
};
