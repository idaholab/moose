/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
