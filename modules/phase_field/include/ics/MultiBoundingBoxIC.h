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
 * MultiBoundingBoxIC allows setting the initial condition of a value of a field inside and outside
 * multiple bounding boxes. Each box is axis-aligned and is specified by passing in the x,y,z
 * coordinates of opposite corners. Separate values for each box may be supplied.
 */
class MultiBoundingBoxIC : public InitialCondition
{
public:
  static InputParameters validParams();

  MultiBoundingBoxIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  ///@{ lists of opposite corners
  const std::vector<Point> _c1;
  const std::vector<Point> _c2;
  ///@}

  /// number of boxes
  const unsigned int _nbox;

  /// dimensionality of the mesh
  const unsigned int _dim;

  /// values inside the boxes
  const std::vector<Real> _inside;

  /// values outside the boxes
  const Real _outside;
};
