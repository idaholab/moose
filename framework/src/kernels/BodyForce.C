#include "BodyForce.h"

template<>
InputParameters validParams<BodyForce>()
{
  InputParameters params = validParams<Kernel>();
  params.set<Real>("value")=0.0;
  return params;
}

BodyForce::BodyForce(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _value(_parameters.get<Real>("value"))
  {}

Real
BodyForce::computeQpResidual()
  {
    return _test[_i][_qp]*-_value;
  }

