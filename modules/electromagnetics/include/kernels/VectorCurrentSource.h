#pragma once

#include "VectorKernel.h"

class VectorCurrentSource;

template <>
InputParameters validParams<VectorCurrentSource>();

class VectorCurrentSource : public VectorKernel
{
public:
  VectorCurrentSource(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const Function & _func;

  const Function & _source_real;
  const Function & _source_imag;

  MooseEnum _component;
};
