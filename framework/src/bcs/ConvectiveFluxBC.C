#include "ConvectiveFluxBC.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<ConvectiveFluxBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<Real>("rate")=7500;
  params.set<Real>("initial")=500;
  params.set<Real>("final")=500;
  params.set<Real>("duration")=0.0;
  return params;
}

ConvectiveFluxBC::ConvectiveFluxBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, setIntegratedParam(parameters, true)),
     _initial(_parameters.get<Real>("initial")),
     _final(_parameters.get<Real>("final")),
     _rate(_parameters.get<Real>("rate")),
     _duration(_parameters.get<Real>("duration"))
{}
 

Real
ConvectiveFluxBC::computeQpResidual()
{
  Real value;

  if(_moose_system._t < _duration)
    value = _initial + (_final-_initial) * std::sin((0.5/_duration) * libMesh::pi * _moose_system._t);
  else
    value = _final;
    
  return -(_phi[_i][_qp]*_rate*(value - _u[_qp]));
}
  
Real
ConvectiveFluxBC::computeQpJacobian()
{
  return -(_phi[_i][_qp]*_rate*(-_phi[_j][_qp]));
}
