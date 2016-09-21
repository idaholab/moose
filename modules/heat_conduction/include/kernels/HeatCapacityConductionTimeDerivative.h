/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEATCAPACITYCONDUCTIONTIMEDERIVATIVE_H
#define HEATCAPACITYCONDUCTIONTIMEDERIVATIVE_H

// MOOSE includes
#include "TimeDerivative.h"
#include "Material.h"

// Forward Declarations
class HeatCapacityConductionTimeDerivative;

template<>
InputParameters validParams<HeatCapacityConductionTimeDerivative>();

/**
 * A class for defining the time derivative of the heat equation.
 *
 * By default this Kernel computes:
 *   \f$ \rho * C_p * \frac{\partial T}{\partial t}, \f$
 * where \f$ C_p \f$ is material property for the "heat_capacity".
 */
class HeatCapacityConductionTimeDerivative : public TimeDerivative
{
public:
  HeatCapacityConductionTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const MaterialProperty<Real> & _heat_capacity;
};

#endif //HEATCAPACITYCONDUCTIONTIMEDERIVATIVE_H
