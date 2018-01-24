/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef DARCYFLUXCOMPONENT_H
#define DARCYFLUXCOMPONENT_H

#include "AuxKernel.h"

// Forward Declarations
class DarcyFluxComponent;

template <>
InputParameters validParams<DarcyFluxComponent>();

/**
 * Computes a component of the Darcy flux:
 * -k_ij/mu (nabla_j P - w_j)
 * where k_ij is the permeability tensor,
 * mu is the fluid viscosity,
 * P is the fluid pressure (the variable)
 * and w_j is the fluid weight
 * This is measured in m^3 . s^-1 . m^-2
 *
 * Sometimes the fluid velocity is required,
 * rather than the flux.  In this case
 * velocity_scaling may be used, and the
 * result quoted above is multiplied by
 * (1/velocity_scaling)
 */
class DarcyFluxComponent : public AuxKernel
{
public:
  DarcyFluxComponent(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// gradient of the pressure
  const VariableGradient & _grad_pp;

  /// fluid weight (gravity*density) as a vector pointing downwards, eg '0 0 -10000'
  RealVectorValue _fluid_weight;

  /// fluid dynamic viscosity
  Real _fluid_viscosity;

  /// (1/velocity_scaling)
  Real _poro_recip;

  /// Material permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  /// Desired component
  unsigned int _component;
};

#endif // DARCYFLUXCOMPONENT_H
