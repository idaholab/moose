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

class VaporMixtureFluidProperties;
class Function;

/**
 * IC for various variables for FlowModelGasMix.
 */
class FlowModelGasMixIC : public InitialCondition
{
public:
  static InputParameters validParams();

  FlowModelGasMixIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p) override;

  /// Quantity type
  enum class Quantity
  {
    DENSITY,
    RHOEA
  };
  /// Which quantity to compute
  const Quantity _quantity;

  /// Secondary gas mass fraction
  const Function & _xi;
  /// Pressure
  const Function & _p;
  /// Temperature
  const Function & _T;
  /// Velocity
  const Function & _vel;
  /// Cross-sectional area
  const VariableValue & _area;

  /// Fluid properties
  const VaporMixtureFluidProperties & _fp;
};
