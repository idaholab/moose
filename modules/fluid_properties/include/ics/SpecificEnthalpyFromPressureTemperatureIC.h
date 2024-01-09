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
 * Computes specific enthalpy from pressure and temperature variables
 */
class SpecificEnthalpyFromPressureTemperatureIC : public InitialCondition
{
public:
  SpecificEnthalpyFromPressureTemperatureIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);

  /// Single phase fluid property user object
  const SinglePhaseFluidProperties & _spfp;
  /// The pressure
  const VariableValue & _p;
  /// The temperature
  const VariableValue & _T;

public:
  static InputParameters validParams();
};
