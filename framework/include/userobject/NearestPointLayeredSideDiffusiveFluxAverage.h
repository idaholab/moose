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
#include "SideIntegralVariableUserObject.h"
#include "NearestPointBase.h"
#include "LayeredSideDiffusiveFluxAverage.h"

/**
 * This UserObject computes averages of a variable's flux storing partial
 * sums for the specified number of intervals in a direction (x,y,z).
 *
 * Given a list of points this object computes the layered average flux
 * closest to each one of those points.
 */
class NearestPointLayeredSideDiffusiveFluxAverage
  : public NearestPointBase<LayeredSideDiffusiveFluxAverage, SideIntegralVariableUserObject>
{
public:
  static InputParameters validParams();

  NearestPointLayeredSideDiffusiveFluxAverage(const InputParameters & parameters);
};
