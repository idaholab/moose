//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

/**
 * BoundingBoxIC allows setting the initial condition of a value inside and outside of a specified
 * box. The box is aligned with the x, y, z axes and is specified by passing in the x, y, z
 * coordinates of the bottom left point and the top right point. Each of the coordinates of the
 * "bottom_left" point MUST be less than those coordinates in the "top_right" point. When setting
 * the initial condition if bottom_left <= Point <= top_right then the "inside" value is used.
 * Otherwise the "outside" value is used.
 */
class BoundingBoxIC : public InitialCondition
{
public:
  static InputParameters validParams();

  BoundingBoxIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  ///@{
  /// The individual components of the bottom left point of the axis aligned bounding box
  const Real _x1;
  const Real _y1;
  const Real _z1;
  ///@}

  ///@{
  /// The individual components of the upper right point of the axis aligned bounding box
  const Real _x2;
  const Real _y2;
  const Real _z2;
  ///@}

  /// The constant value to assign within the bounding box
  const Real _inside;

  /// The constant value to assign outside of the bounding box
  const Real _outside;

  /// The Point object constructed from the x1, y1, z1 components for the bottom left BB corner
  const Point _bottom_left;

  /// The Point object constructed from the x2, y2, z2 components for the bottom left BB corner
  const Point _top_right;

  /// Interfacial width
  const Real _int_width;
};
