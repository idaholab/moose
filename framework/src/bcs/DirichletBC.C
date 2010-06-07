#include "DirichletBC.h"

template<>
InputParameters validParams<DirichletBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<Real>("value")=0.0;
  return params;
}

DirichletBC::DirichletBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, setIntegratedParam(parameters, false)),
    _value(_parameters.get<Real>("value"))
  {}

Real
DirichletBC::computeQpResidual()
  {
    return _u[_qp]-_value;
  }
