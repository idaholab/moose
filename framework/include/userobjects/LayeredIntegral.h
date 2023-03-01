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
#include "LayeredBase.h"

/**
 * This UserObject computes volume integrals of a variable storing partial sums for the specified
 * number of intervals in a direction (x,y,z).
 */
class LayeredIntegral : public ElementIntegralVariableUserObject, public LayeredBase
{
public:
  static InputParameters validParams();

  LayeredIntegral(const InputParameters & parameters);

  /**
   * Given a Point return the integral value associated with the layer that point falls in.
   *
   * @param p The point to look for in the layers.
   */
  virtual Real spatialValue(const Point & p) const override { return integralValue(p); }

  virtual const std::vector<Point> spatialPoints() const override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;
};
