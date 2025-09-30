//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SlopeLimitingMultiDBase.h"

/**
 * Venkatakrishnan multi-D slope limiter with smooth beta parameter.
 *
 * Provides smoother limiting than Barth–Jespersen for better Newton behavior.
 */
class SlopeLimitingVenkatakrishnan : public SlopeLimitingMultiDBase
{
public:
  static InputParameters validParams();

  SlopeLimitingVenkatakrishnan(const InputParameters & parameters);
  virtual ~SlopeLimitingVenkatakrishnan() {}

  virtual std::vector<libMesh::RealGradient> limitElementSlope() const override;

protected:
  const Real _beta;                 // smoothing parameter
  const bool _couple_momentum_to_h; // if true, apply phi_h to hu,hv as well
};

