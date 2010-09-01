#include "DirichletBC.h"

template<>
InputParameters validParams<DirichletBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addParam<Real>("value", 0.0, "The value the variable should have on the boundary");
  params.set<bool>("_integrated") = false;
  return params;
}

DirichletBC::DirichletBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
    _value(getParam<Real>("value"))
{}

Real
DirichletBC::computeQpResidual()
{
  return _u[_qp]-_value;
}
