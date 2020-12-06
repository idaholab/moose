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
 * SmoothMultiBoundingBoxBaseIC is the base class for IsolatedBoundingBoxIC and NestedBoundingBoxIC.
 * The boxes can finite interface width.
 */
class SmoothMultiBoundingBoxBaseIC : public InitialCondition
{
public:
  static InputParameters validParams();

  SmoothMultiBoundingBoxBaseIC(const InputParameters & parameters);

  Real value(const Point & p);

private:
  const Real _outside;

protected:
  ///@{ lists of opposite corners
  const std::vector<Point> _c1;
  const std::vector<Point> _c2;
  ///@}

  /// number of boxes
  const unsigned int _nbox;

  /// value of interfacial width
  const Real _int_width;

  /// dimensionality of the mesh
  const unsigned int _dim;

  /// values inside the boxes
  std::vector<Real> _inside;
};
