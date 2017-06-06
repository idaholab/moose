/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONSISTENTSPECIFICHEATTIMEDERIVATIVE_H
#define CONSISTENTSPECIFICHEATTIMEDERIVATIVE_H

// MOOSE includes
#include "SpecificHeatConductionTimeDerivative.h"

// Forward Declarations
class ConsistentSpecificHeatTimeDerivative;

template <>
InputParameters validParams<ConsistentSpecificHeatTimeDerivative>();

/**
 * A class for defining the time derivative of the heat equation.
 *
 * By default this Kernel computes:
 *   \f$ (c_p * rho + c_p * T * \frac{\partial rho}{\partial T}
 *   + rho * T * \frac{\partial c_p}{\partial T}) * \frac{\partial T}{\partial
 * t}, \f$
 * where \f$ c_p \f$ is material property for the "specific_heat" and
 *       \f$ rho \f$ is the material property for the density.
 */
class ConsistentSpecificHeatTimeDerivative : public SpecificHeatConductionTimeDerivative
{
public:
  ConsistentSpecificHeatTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
};

#endif // ConsistentSpecificHeatTimeDerivative_H
