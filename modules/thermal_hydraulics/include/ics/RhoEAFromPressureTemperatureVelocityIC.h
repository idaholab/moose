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

class SinglePhaseFluidProperties;

/**
 * The RhoEAFromPressureTemperatureVelocityIC returns:
 *
 * rho * e(p, rho) * A + 0.5 * rho * velocity * velocity * A
 *
 */
class RhoEAFromPressureTemperatureVelocityIC : public InitialCondition
{
public:
  RhoEAFromPressureTemperatureVelocityIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);

  const SinglePhaseFluidProperties & _fp;
  /// The pressure
  const VariableValue & _p;
  /// The temperature
  const VariableValue & _T;
  /// The velocity
  const VariableValue & _vel;
  /// Cross-sectional area
  const VariableValue & _area;

public:
  static InputParameters validParams();
};
