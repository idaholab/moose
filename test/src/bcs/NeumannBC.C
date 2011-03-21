#include "NeumannBC.h"

template<>
InputParameters validParams<NeumannBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<Real>("value", 0.0, "The value of the gradient on the boundary.");
  return params;
}

NeumannBC::NeumannBC(const std::string & name, InputParameters parameters) :
  IntegratedBC(name, parameters),
  _value(parameters.get<Real>("value"))
{
}

Real
NeumannBC::computeQpResidual()
{
  return -_phi[_i][_qp]*_value;
}

