//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegralRayKernelBase.h"

/**
 * Base class for a RayKernel that integrates along a Ray segment and stores the result in a scalar
 * value on the Ray
 */
class IntegralRayKernel : public IntegralRayKernelBase
{
public:
  IntegralRayKernel(const InputParameters & params);

  static InputParameters validParams();

  /**
   * Gets the name of the Ray data associated with the integral accumulated by this RayKernel
   */
  std::string integralRayDataName() const { return _name + "_value"; }

  void onSegment() override final;

protected:
  virtual Real computeQpIntegral() = 0;

  /// The index into the data on the Ray that this integral accumulates into
  const RayDataIndex _integral_data_index;

  /// Whether or not to compute the average (divide by the length)
  const bool _average;
};
