//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ConservedNoiseBase.h"
#include "ConservedNormalNoiseVeneer.h"

// Forward delcarations

/**
 * Userobject that generates a normaly distributed random number
 * once per timestep for every quadrature point in a way that the integral
 * over all random numbers is zero.
 *
 * \see ConservedNoiseBase
 */
class ConservedNormalNoise : public ConservedNormalNoiseVeneer<ConservedNoiseBase>
{
public:
  static InputParameters validParams();

  ConservedNormalNoise(const InputParameters & parameters)
    : ConservedNormalNoiseVeneer<ConservedNoiseBase>(parameters)
  {
  }
};
