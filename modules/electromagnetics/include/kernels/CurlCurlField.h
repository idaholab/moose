#ifndef CURLCURLFIELD_H
#define CURLCURLFIELD_H

#include "VectorKernel.h"

class CurlCurlField;

template <>
InputParameters validParams<CurlCurlField>();

class CurlCurlField : public VectorKernel
{
public:
  CurlCurlField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  Real _sign;
};

#endif // CURLCURLFIELD_H
