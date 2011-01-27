#include "Gravity.h"

template<>
InputParameters validParams<Gravity>()
{
  InputParameters params = validParams<BodyForce>();
  return params;
}

Gravity::Gravity(const std::string & name, InputParameters parameters)
  :BodyForce(name, parameters),
   _density(getMaterialProperty<Real>("density"))
{}

Real
Gravity::computeQpResidual()
{
  return _density[_qp] * BodyForce::computeQpResidual();
}

