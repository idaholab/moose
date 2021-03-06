#pragma once

#include "VectorKernel.h"

class VectorCoeffField : public VectorKernel
{
public:
  static InputParameters validParams();

  VectorCoeffField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  Real _coefficient;

  const Function & _func;
};
