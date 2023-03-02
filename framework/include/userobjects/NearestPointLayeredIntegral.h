//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ElementIntegralVariableUserObject.h"
#include "NearestPointBase.h"
#include "LayeredIntegral.h"

/**
 * This UserObject computes integrals of a variable storing partial
 * sums for the specified number of intervals in a direction (x,y,z).
 *
 * Given a list of points this object computes the layered integral
 * closest to each one of those points.
 */
class NearestPointLayeredIntegral
  : public NearestPointBase<LayeredIntegral, ElementIntegralVariableUserObject>
{
public:
  static InputParameters validParams();

  NearestPointLayeredIntegral(const InputParameters & parameters);
};
