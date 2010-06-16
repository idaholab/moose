#include "SinDirichletBC.h"
#include "MooseSystem.h"
 
template<>
InputParameters validParams<SinDirichletBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<Real>("initial")=0.0;
  params.set<Real>("final")=0.0;
  params.set<Real>("duration")=0.0;

  params.set<bool>("_integrated") = false;
  return params;
}

SinDirichletBC::SinDirichletBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
   _initial(_parameters.get<Real>("initial")),
   _final(_parameters.get<Real>("final")),
   _duration(_parameters.get<Real>("duration"))
{
}

Real
SinDirichletBC::computeQpResidual()
{
  Real value;

  if(_moose_system._t < _duration)
    value = _initial + (_final-_initial) * std::sin((0.5/_duration) * libMesh::pi * _moose_system._t);
  else
    value = _final;
  
  return _u[_qp]- value;
}
