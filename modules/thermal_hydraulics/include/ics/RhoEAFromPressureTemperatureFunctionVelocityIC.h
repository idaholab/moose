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
class Function;

/**
 * This IC sets total energy density from provided pressure, temperature and user-defined veloctiy
 * function
 *
 * It computes:
 * \f[
 *  \rho E A = \rho e(p, \rho) A + 0.5 \rho u^2 A
 * \f]
 */
class RhoEAFromPressureTemperatureFunctionVelocityIC : public InitialCondition
{
public:
  RhoEAFromPressureTemperatureFunctionVelocityIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);

  const SinglePhaseFluidProperties & _fp;
  /// The pressure
  const VariableValue & _p;
  /// The temperature
  const VariableValue & _T;
  /// The velocity given as a function
  const Function & _vel;
  /// Cross-sectional area
  const VariableValue & _area;

public:
  static InputParameters validParams();
};
