#pragma once

#include "AuxKernel.h"

/**
 * Computes the component of a vector (given by its magnitude and direction)
 */
class ADVectorVelocityComponentAux : public AuxKernel
{
public:
  ADVectorVelocityComponentAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Solution variable arhoA
  const VariableValue & _arhoA;
  /// Solution variable arhouA
  const VariableValue & _arhouA;
  /// Direction
  const ADMaterialProperty<RealVectorValue> & _dir;
  /// Vector component to use
  const unsigned int _component;

public:
  static InputParameters validParams();
};
