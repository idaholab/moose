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
#include "LayeredBase.h"

/**
 * This layered user object computes the change in cross sectional area
 * of a flow channel
 */
class LayeredFlowAreaChange : public SideIntegralUserObject, public LayeredBase
{
public:
  LayeredFlowAreaChange(const InputParameters & parameters);

  /**
   * Given a Point return the integral value associated with the layer that point falls in.
   *
   * @param p The point to look for in the layers.
   */
  virtual Real spatialValue(const Point & p) const override { return integralValue(p); }

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual Real computeIntegral() override;

protected:
  virtual Real computeQpIntegral() override;

  /// the problem dimension
  unsigned int _dim;

  /// the displacement vectors
  std::vector<const VariableValue *> _disp;

public:
  static InputParameters validParams();
};
