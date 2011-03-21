#include "DirichletBC.h"

template<>
InputParameters validParams<DirichletBC>()
{
  InputParameters p = validParams<NodalBC>();
  p.addRequiredParam<Real>("value", "Value of the BC");
  return p;
}


DirichletBC::DirichletBC(const std::string & name, InputParameters parameters) :
  NodalBC(name, parameters),
  _value(parameters.get<Real>("value"))
{

}

DirichletBC::~DirichletBC()
{
}

Real
DirichletBC::computeQpResidual()
{
  return _u[_qp] - _value;
}

