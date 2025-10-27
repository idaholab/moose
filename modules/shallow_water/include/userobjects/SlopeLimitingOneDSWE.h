//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SlopeLimitingBase.h"

/**
 * 1D TVD MUSCL slope limiter for SWE variables [h, hu, hv] on uniform x-aligned meshes.
 *
 * - Computes limited slopes in x using left/right neighbor cell averages.
 * - Supports schemes: none, minmod, mc, superbee.
 * - Returns a vector<RealGradient> of size 3 for [h, hu, hv]; only x-component is nonzero.
 * - Intended for 1D comparisons; on elements with n_sides != 2, returns zero slopes.
 */
class SlopeLimitingOneDSWE : public SlopeLimitingBase
{
public:
  static InputParameters validParams();

  SlopeLimitingOneDSWE(const InputParameters & parameters);

  virtual std::vector<libMesh::RealGradient> limitElementSlope() const override;

protected:
  MooseVariable * _hvar;
  MooseVariable * _huvar;
  MooseVariable * _hvvar;
  const MooseEnum _scheme;
};
