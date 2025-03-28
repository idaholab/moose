//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class VaporMixtureFluidProperties;

/**
 * Computes various quantities for FlowModelGasMix.
 */
class FlowModelGasMixAux : public AuxKernel
{
public:
  static InputParameters validParams();

  FlowModelGasMixAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Quantity type
  enum class Quantity
  {
    PRESSURE,
    TEMPERATURE
  };
  /// Which quantity to compute
  const Quantity _quantity;

  const VariableValue & _xirhoA;
  const VariableValue & _rhoA;
  const VariableValue & _rhouA;
  const VariableValue & _rhoEA;
  const VariableValue & _area;

  /// Fluid properties
  const VaporMixtureFluidProperties & _fp;
};
