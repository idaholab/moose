//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralRayKernel.h"

/**
 * Simplified RayKernel that refracts Rays when the phase changes (phase change denoted by a field
 * variable). The initial purpose of this RayKernel is to test the act of changing a Ray's start
 * point and direction mid-segment (not the physics!).
 *
 * The following assumptions are made:
 * - The ray refracts only once and this happens the first time the phase variable is not 0 or 1 at
 * the midpoint of the ray segment in an element
 * - The refraction happens at the midpoint of said segment
 * - The indices of refraction are not associated with a single phase, that is, _r1 is always used
 * as the incident index of refraction
 */

class RefractionRayKernelTest : public GeneralRayKernel
{
public:
  RefractionRayKernelTest(const InputParameters & params);

  static InputParameters validParams();

  virtual void onSegment() override;

protected:
  /**
   * Computes the refracted direction using Snell's law
   * @param direction The incident direction
   * @param normal The normal of the interface
   * @param r1 Index of refraction for the incident material
   * @param r2 Index of refraction for the outgoing material
   * @return The refracted direction
   */
  Point refract(const Point & direction, const Point & normal, const Real r1, const Real r2) const;

  /// The field variable that notes the phase
  const VariableValue & _field;
  /// The gradient of the field variable that notes the phase
  const VariableGradient & _grad_field;

  /// The first index of refraction
  const Real _r1;
  /// The second index of refraction
  const Real _r2;

  /// Index of the Ray data that notes if the Ray has refracted or not
  const RayDataIndex _has_refracted_data_index;
};
