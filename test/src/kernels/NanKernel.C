#include "NanKernel.h"

template<>
InputParameters validParams<NanKernel>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}


NanKernel::NanKernel(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters)
{
}

Real
NanKernel::computeQpResidual()
{
  double x = 0.;
  double y = 0.;

  x = x / y;
  return x;
}

Real
NanKernel::computeQpJacobian()
{
  return 0;
}
