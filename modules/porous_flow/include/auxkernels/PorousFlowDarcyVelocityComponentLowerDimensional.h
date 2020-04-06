//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowDarcyVelocityComponent.h"

/**
 * Computes a component of the Darcy velocity:
 * -k_ij * krel /(mu a) (nabla_j P - w_j)
 * where k_ij is the permeability tensor,
 * krel is the relative permeaility,
 * mu is the fluid viscosity,
 * a is the fracture aperture,
 * P is the fluid pressure
 * and w_j is the fluid weight tensor that is projected in the tangent direction of this element
 * This is measured in m^3 . s^-1 . m^-2
 */
class PorousFlowDarcyVelocityComponentLowerDimensional : public PorousFlowDarcyVelocityComponent
{
public:
  static InputParameters validParams();

  PorousFlowDarcyVelocityComponentLowerDimensional(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Fracture aperture (width)
  const VariableValue & _aperture;
};
