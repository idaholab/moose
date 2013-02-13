#include "ExceptionKernel.h"
#include "MooseException.h"

template<>
InputParameters validParams<ExceptionKernel>()
{
  InputParameters params = validParams<Kernel>();
  MooseEnum when("residual = 0, jacobian, initial_condition", "residual");
  params.addParam<MooseEnum>("when", when, "When to throw the exception");
  return params;
}


ExceptionKernel::ExceptionKernel(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _when(static_cast<WhenType>((int) getParam<MooseEnum>("when"))),
    _call_no(0)
{
}

Real
ExceptionKernel::computeQpResidual()
{
  if (_when == INITIAL_CONDITION)
    throw MooseException(1);
  else if (_when == RESIDUAL)
  {
    if (_call_no == 1000)                 // 1000 calls to computeQpResidual is enough to get us into linear solve
      throw MooseException(2);
  }
  _call_no++;

  return 0.;
}

Real
ExceptionKernel::computeQpJacobian()
{
  if (_when == JACOBIAN)
    throw MooseException(3);

  return 0.;
}
