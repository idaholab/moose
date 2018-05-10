#ifndef VECTORCURRENTSOURCE_H
#define VECTORCURRENTSOURCE_H

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

  Real _coefficient;

  Function & _func;

  RealVectorValue _source_real;

  RealVectorValue _source_imaginary;

  MooseEnum _component;
};

#endif // VECTORCURRENTSOURCE_H
