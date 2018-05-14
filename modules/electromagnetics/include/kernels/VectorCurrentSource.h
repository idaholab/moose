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

  Function & _x_real;
  Function & _y_real;
  Function & _z_real;

  Function & _x_imag;
  Function & _y_imag;
  Function & _z_imag;

  // RealVectorValue _source_real;
  //
  // RealVectorValue _source_imaginary;

  MooseEnum _component;
};

#endif // VECTORCURRENTSOURCE_H
