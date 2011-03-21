#include "ImplicitEuler.h"

template<>
InputParameters validParams<ImplicitEuler>()
{
  InputParameters params = validParams<TimeDerivative>();
  return params;
}

ImplicitEuler::ImplicitEuler(const std::string & name, InputParameters parameters) :
    TimeDerivative(name, parameters)
{
}

Real
ImplicitEuler::computeQpResidual()
{
  return TimeDerivative::computeQpResidual();
}

Real
ImplicitEuler::computeQpJacobian()
{
  return TimeDerivative::computeQpJacobian();
}
