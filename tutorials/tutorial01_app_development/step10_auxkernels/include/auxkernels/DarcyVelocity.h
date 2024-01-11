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
 * Auxiliary kernel responsible for computing the Darcy velocity (discharge per unit area) vector
 * given permeability, viscosity, and the pressure gradient of the medium.
 */
class DarcyVelocity : public VectorAuxKernel
{
public:
  static InputParameters validParams();

  DarcyVelocity(const InputParameters & parameters);

protected:
  /// AuxKernels MUST override computeValue(), which is called on every Gauss QP for elemental
  /// AuxVariables. For nodal AuxVariables, it is called on every node instead and the _qp index
  /// automatically refers to those nodes.
  virtual RealVectorValue computeValue() override;

  /// The gradient of a coupled variable, i.e., pressure
  const VariableGradient & _grad_p;

  /// The material properties which hold the values for K and mu
  const ADMaterialProperty<Real> & _permeability;
  const ADMaterialProperty<Real> & _viscosity;
};
