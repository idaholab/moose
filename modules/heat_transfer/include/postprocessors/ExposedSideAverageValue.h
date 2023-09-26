//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideAverageValue.h"

class SelfShadowSideUserObject;

/**
 * This postprocessor computes a volume integral of the specified variable
 * on the exposed portion of a surface.
 */
class ExposedSideAverageValue : public SideAverageValue
{
public:
  static InputParameters validParams();

  ExposedSideAverageValue(const InputParameters & parameters);
  Real computeQpIntegral() override;

protected:
  /**
   * Instead of computing the total face volume (as the base class does), compute
   * the volume of the exposed QPs on that face.
   * @return exposed face volume
   */
  virtual Real volume() override;

  /// Reference to SelfShadowSideUserObject, which does the determination of which
  /// QPs are exposed.
  const SelfShadowSideUserObject & _self_shadow;
};
