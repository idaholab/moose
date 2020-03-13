//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"

/**
 * This class computes the volume of an interior space.
 *
 * This class relies on being handed sidesets that define a closed space.
 *
 * If the sideset defines an interior surface, the volume reported will be
 * positive.  If the sideset defines an exterior surface, the volume
 * reported will be negative.  It is therefore possible to compute the net
 * interior volume by including an interior and an exterior surface
 * in the same sideset.
 */
class InternalVolume : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  InternalVolume(const InputParameters & parameters);

  void initialSetup();

protected:
  virtual Real computeQpIntegral();
  virtual Real getValue();

  const unsigned int _component;
  const Real _scale;
  const Function & _addition;
};
