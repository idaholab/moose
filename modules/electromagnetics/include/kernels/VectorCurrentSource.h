#pragma once

#include "VectorKernel.h"

class VectorCurrentSource : public VectorKernel
{
public:
  static InputParameters validParams();

  VectorCurrentSource(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const Function & _func;

  const Function & _source_real;
  const Function & _source_imag;

  MooseEnum _component;
};
