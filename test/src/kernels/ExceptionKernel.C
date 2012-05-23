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
  // if the exception occurs only on one processor, we need to propagate it on the other ones as well
  parallel_if(libMesh::processor_id() == 0)
    throw MooseException();

  return 0.;
}
