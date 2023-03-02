//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralVariableUserObject.h"
#include "NearestPointBase.h"
#include "LayeredSideIntegral.h"

/**
 * This UserObject computes integrals of a variable storing partial
 * sums for the specified number of intervals in a direction (x,y,z).
 *
 * Given a list of points this object computes the layered integral
 * closest to each one of those points.
 */
class NearestPointLayeredSideIntegral
  : public NearestPointBase<LayeredSideIntegral, SideIntegralVariableUserObject>
{
public:
  static InputParameters validParams();

  NearestPointLayeredSideIntegral(const InputParameters & parameters);
};
