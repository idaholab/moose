#ifndef COUPLEDCURLCURLFIELD_H
#define COUPLEDCURLCURLFIELD_H

#include "VectorKernel.h"

class CoupledCurlCurlField;

template <>
InputParameters validParams<CoupledCurlCurlField>();

class CoupledCurlCurlField : public VectorKernel
{
public:
  CoupledCurlCurlField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  Real _sign;

  const VectorVariableCurl & _coupled_curl;
};

#endif // COUPLEDCURLCURLFIELD_H
