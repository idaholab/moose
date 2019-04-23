//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IdealGasFluidProperties.h"

class IdealGasFluidPropertiesPT;

template <>
InputParameters validParams<IdealGasFluidPropertiesPT>();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Ideal gas fluid properties for (pressure, temperature) variables.
 * Default parameters are for air at atmospheric pressure and temperature.
 */
class IdealGasFluidPropertiesPT : public IdealGasFluidProperties
{
public:
  IdealGasFluidPropertiesPT(const InputParameters & parameters);
};

#pragma GCC diagnostic pop

