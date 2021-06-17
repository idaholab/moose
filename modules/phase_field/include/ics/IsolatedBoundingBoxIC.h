//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SmoothMultiBoundingBoxBaseIC.h"

/**
 * IsolatedBoundingBoxIC creates several isolated boxes defined by their coordinates in the domain.
 * If int_width > zero, the border of the boxes smoothly transitions from
 * the invalue to the outvalue.
 */
class IsolatedBoundingBoxIC : public SmoothMultiBoundingBoxBaseIC
{
public:
  static InputParameters validParams();

  IsolatedBoundingBoxIC(const InputParameters & parameters);

  Real value(const Point & p);

protected:
  /// values outside all the boxes
  const Real _outside;
};
