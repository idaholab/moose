//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

class SinglePhaseFluidProperties;

/**
 * Computes the density using the fluid properties at a specified location
 */
class RhoFromPTFunctorMaterial : public FunctorMaterial
{
public:
  RhoFromPTFunctorMaterial(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  /// pressure
  const MooseVariableFVReal & _pressure;

  /// temperature
  const MooseVariableFVReal & _temperature;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fluid;
};
