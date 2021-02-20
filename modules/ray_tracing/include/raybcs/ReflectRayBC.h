//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralRayBC.h"

/**
 * RayBC that reflects a Ray
 */
class ReflectRayBC : public GeneralRayBC
{
public:
  ReflectRayBC(const InputParameters & params);

  static InputParameters validParams();

  virtual void onBoundary(const unsigned int num_applying) override;

  /**
   * Computes the reflected direction given a direction and an outward normal for the surface it
   * reflects off of.
   */
  static Point reflectedDirection(const Point & direction, const Point & normal);

protected:
  /// Whether or not to emit a warning if a Ray is being reflected on a non-planar side
  const bool _warn_non_planar;
};
