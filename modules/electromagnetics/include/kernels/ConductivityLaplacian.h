#pragma once

#include "Kernel.h"

class ConductivityLaplacian;

template <>
InputParameters validParams<ConductivityLaplacian>();

class ConductivityLaplacian : public Kernel
{
public:
  static InputParameters validParams();

  ConductivityLaplacian(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  const MaterialProperty<Real> & _conductivity;
  const MaterialProperty<Real> * const _conductivity_dT;
};
