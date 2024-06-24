//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Auxiliary kernel responsible for computing the Darcy velocity given
 * several fluid properties and the pressure gradient.
 */
class DarcyVelocity : public VectorAuxKernel
{
public:
  static InputParameters validParams();

  DarcyVelocity(const InputParameters & parameters);

protected:
  /**
   * AuxKernels MUST override computeValue.  computeValue() is called on
   * every quadrature point.  For Nodal Auxiliary variables those quadrature
   * points coincide with the nodes.
   */
  virtual RealVectorValue computeValue() override;

  /// The gradient of a coupled variable
  const VariableGradient & _pressure_gradient;

  /// Holds the permeability and viscosity from the material system
  const ADMaterialProperty<Real> & _permeability;
  const ADMaterialProperty<Real> & _viscosity;
};
