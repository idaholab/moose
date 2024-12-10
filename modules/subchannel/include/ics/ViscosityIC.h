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
 * Computes initial viscosity from specified pressure and temperature
 */
class ViscosityIC : public InitialCondition
{
public:
  static InputParameters validParams();

  ViscosityIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// Temperature
  const VariableValue & _T;
  /// Pressure
  const Real & _P;
  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;
};
