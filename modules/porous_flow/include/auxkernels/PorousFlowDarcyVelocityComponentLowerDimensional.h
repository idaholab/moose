/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDARCYVELOCITYCOMPONENTLOWERDIMENSIONAL_H
#define POROUSFLOWDARCYVELOCITYCOMPONENTLOWERDIMENSIONAL_H

#include "PorousFlowDarcyVelocityComponent.h"

// Forward Declarations
class PorousFlowDarcyVelocityComponentLowerDimensional;

template <>
InputParameters validParams<PorousFlowDarcyVelocityComponentLowerDimensional>();

/**
 * Computes a component of the Darcy velocity:
 * -k_ij * krel /mu (nabla_j P - w_j)
 * where k_ij is the permeability tensor,
 * krel is the relative permeaility,
 * mu is the fluid viscosity,
 * P is the fluid pressure
 * and w_j is the fluid weight tensor that is projected in the tangent direction of this element
 * This is measured in m^3 . s^-1 . m^-2
 */
class PorousFlowDarcyVelocityComponentLowerDimensional : public PorousFlowDarcyVelocityComponent
{
public:
  PorousFlowDarcyVelocityComponentLowerDimensional(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
};

#endif // POROUSFLOWDARCYVELOCITYCOMPONENTLOWERDIMENSIONAL_H
