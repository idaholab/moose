//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "LevelSetContactor.h"

/**
 * Rigid contactor for an infinite cylinder of the given radius, whose axis
 * passes through `origin` in the direction `axis` (any nonzero direction;
 * normalized in the constructor).  Signed distance is the perpendicular
 * distance to the axis minus the radius, so g_LS > 0 outside the cylinder
 * and g_LS < 0 inside it -- the same convention every other LevelSetContactor
 * subclass uses.  Suitable for rolling-mill / roller-contact setups where the
 * roller can be treated as a straight, arbitrarily long circular cylinder.
 */
class InfiniteCylinderContactor : public LevelSetContactor
{
public:
  static InputParameters validParams();
  InfiniteCylinderContactor(const InputParameters &);

protected:
  virtual Real signedDistanceRaw(const Point &) const override;
  virtual RealVectorValue normalRaw(const Point &) const override;
  virtual RealTensorValue hessianRaw(const Point &) const override;

  /// A point on the cylinder's axis (any point suffices).
  const Point _origin;
  /// Unit vector along the axis (user input is normalized in the ctor).
  RealVectorValue _axis;
  /// Cylinder radius.
  const Real _radius;
};
