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

  Function & _func;

  Function & _source_real;
  Function & _source_imag;

  MooseEnum _component;
};

#endif // VECTORCURRENTSOURCE_H
