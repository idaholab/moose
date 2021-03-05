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
 * Compute the a component of the second moment of the area of a sideset w.r.t. a point
 */
class PolarMomentOfInertia : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  PolarMomentOfInertia(const InputParameters & parameters);

protected:
  Real computeQpIntegral() override;

  const Point _origin;
  RealVectorValue _direction;
};
