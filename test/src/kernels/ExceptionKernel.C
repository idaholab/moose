#include "ExceptionKernel.h"
#include "MooseException.h"

template<>
InputParameters validParams<ExceptionKernel>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}


ExceptionKernel::ExceptionKernel(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters)
{
}

Real
ExceptionKernel::computeQpResidual()
{
  return 0.;
}

Real
ExceptionKernel::computeQpJacobian()
{
  throw MooseException();
  return 0.;
}
