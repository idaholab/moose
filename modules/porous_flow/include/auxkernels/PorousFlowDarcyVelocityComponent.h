/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDARCYVELOCITYCOMPONENT_H
#define POROUSFLOWDARCYVELOCITYCOMPONENT_H

#include "AuxKernel.h"
#include "PorousFlowDictator.h"

// Forward Declarations
class PorousFlowDarcyVelocityComponent;

template <>
InputParameters validParams<PorousFlowDarcyVelocityComponent>();

/**
 * Computes a component of the Darcy velocity:
 * -k_ij * krel /mu (nabla_j P - w_j)
 * where k_ij is the permeability tensor,
 * krel is the relative permeaility,
 * mu is the fluid viscosity,
 * P is the fluid pressure
 * and w_j is the fluid weight
 * This is measured in m^3 . s^-1 . m^-2
 */
class PorousFlowDarcyVelocityComponent : public AuxKernel
{
public:
  PorousFlowDarcyVelocityComponent(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real>> & _relative_permeability;

  /// Viscosity of each component in each phase
  const MaterialProperty<std::vector<Real>> & _fluid_viscosity;

  /// Permeability of porous material
  const MaterialProperty<RealTensorValue> & _permeability;

  /// Gradient of the pore pressure in each phase
  const MaterialProperty<std::vector<RealGradient>> & _grad_p;

  /// Fluid density for each phase (at the qp)
  const MaterialProperty<std::vector<Real>> & _fluid_density_qp;

  /// PorousFlow UserObject
  const PorousFlowDictator & _dictator;

  /// Index of the fluid phase
  const unsigned int _ph;

  /// Desired spatial component
  unsigned int _component;

  /// Gravitational acceleration
  const RealVectorValue _gravity;
};

#endif // POROUSFLOWDARCYVELOCITYCOMPONENT_H
