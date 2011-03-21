#include "Coupled.h"

template<>
InputParameters validParams<Coupled>()
{
  return validParams<Kernel>();
}


Coupled::Coupled(const std::string & name, InputParameters parameters) :
  Kernel(name, parameters),
  _v(coupledValue(parameters.get<std::string>("coupled_var_name")))
{

}

Coupled::~Coupled()
{

}

Real
Coupled::computeQpResidual()
{
  return _v[_qp] * _test[_i][_qp];
}
