#include "BodyForce.h"

template<>
InputParameters validParams<BodyForce>()
{
  InputParameters params = validParams<Kernel>();
  params.set<Real>("value")=0.0;
  return params;
}

BodyForce::BodyForce(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _value(getParam<Real>("value"))
{
}

Real
BodyForce::computeQpResidual()
{
  return _test[_i][_qp]*-_value;
}
