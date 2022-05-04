//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VariableGradientComponent.h"

/**
 *  This AuxKernel calculates the electrostatic electric field given the
 *  electrostatic potential.
 */
class PotentialToFieldAux : public VariableGradientComponent
{
public:
  static InputParameters validParams();

  PotentialToFieldAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Sign prefactor for calculation as determined via MooseEnum (default = negative, or -1)
  Real _sign;
};
