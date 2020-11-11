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
