//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONSISTENTHEATCAPACITYTIMEDERIVATIVE_H
#define CONSISTENTHEATCAPACITYTIMEDERIVATIVE_H

// MOOSE includes
#include "HeatCapacityConductionTimeDerivative.h"

// Forward Declarations
class ConsistentHeatCapacityTimeDerivative;

template <>
InputParameters validParams<ConsistentHeatCapacityTimeDerivative>();

/**
 * A class for defining the time derivative of the heat equation.
 *
 * By default this Kernel computes:
 *   \f$ (C_p + T \frac{\partial C_p}{\partial T}) * \frac{\partial T}{\partial
 * t}, \f$
 * where \f$ C_p \f$ is material property for the "heat_capacity".
 */
class ConsistentHeatCapacityTimeDerivative : public HeatCapacityConductionTimeDerivative
{
public:
  ConsistentHeatCapacityTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
};

#endif // ConsistentHeatCapacityTimeDerivative_H
