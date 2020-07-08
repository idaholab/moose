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
#include "VaporMixtureInterface.h"

/**
 * Computes the density of a vapor mixture from pressure and temperature variables
 */
class RhoVaporMixtureFromPressureTemperatureIC : public VaporMixtureInterface<InitialCondition>
{
public:
  RhoVaporMixtureFromPressureTemperatureIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);

  /// Pressure
  const VariableValue & _p;
  /// Temperature
  const VariableValue & _T;

public:
  static InputParameters validParams();
};
