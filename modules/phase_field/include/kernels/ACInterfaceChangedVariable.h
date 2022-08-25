/// Considers cleavage plane anisotropy in the crack propagation

#pragma once

#include "ACInterface.h"

class ACInterfaceChangedVariable : public ACInterface
{
public:
  static InputParameters validParams();

  ACInterfaceChangedVariable(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// Order parameter derivative
  const MaterialProperty<Real> & _dopdu;

  /// 2nd order parameter derivative
  const MaterialProperty<Real> & _d2opdu2;
};
