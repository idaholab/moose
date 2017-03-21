/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef DARCYFLUX
#define DARCYFLUX

#include "Kernel.h"

// Forward Declarations
class DarcyFlux;

template <>
InputParameters validParams<DarcyFlux>();

/**
 * Kernel = grad(permeability*(grad(pressure) - weight))
 * This is mass flow according to the Darcy equation
 */
class DarcyFlux : public Kernel
{
public:
  DarcyFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  /// fluid weight (gravity*density) as a vector pointing downwards, eg '0 0 -10000'
  RealVectorValue _fluid_weight;

  /// fluid dynamic viscosity
  Real _fluid_viscosity;

  /// Material permeability
  const MaterialProperty<RealTensorValue> & _permeability;
};

#endif // DARCYFLUX
