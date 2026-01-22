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
 * Barth-Jespersen multi-dimensional slope limiter.
 * Limits reconstructed linear slopes to avoid creating new extrema across faces.
 */
class SlopeLimitingBarthJespersen : public SlopeLimitingMultiDBase
{
public:
  static InputParameters validParams();

  SlopeLimitingBarthJespersen(const InputParameters & parameters);
  virtual ~SlopeLimitingBarthJespersen() {}

  virtual std::vector<libMesh::RealGradient> limitElementSlope() const override;
};
