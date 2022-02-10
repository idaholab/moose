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
 * Computes the component of a vector (given by its magnitude and direction)
 */
class VectorVelocityComponentAux : public AuxKernel
{
public:
  VectorVelocityComponentAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Solution variable arhoA
  const VariableValue & _arhoA;
  /// Solution variable arhouA
  const VariableValue & _arhouA;
  /// Direction
  const MaterialProperty<RealVectorValue> & _dir;
  /// Vector component to use
  const unsigned int _component;

public:
  static InputParameters validParams();
};
