#include "ConvectiveFluxBC.h"

template<>
InputParameters valid_params<ConvectiveFluxBC>()
{
  InputParameters params;
  params.set<Real>("rate")=7500;
  params.set<Real>("initial")=500;
  params.set<Real>("final")=500;
  params.set<Real>("duration")=0.0;
  return params;
}

ConvectiveFluxBC::ConvectiveFluxBC(std::string name, InputParameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, true, boundary_id, coupled_to, coupled_as),
     _initial(_parameters.get<Real>("initial")),
     _final(_parameters.get<Real>("final")),
     _rate(_parameters.get<Real>("rate")),
     _duration(_parameters.get<Real>("duration"))

{}
 

Real
ConvectiveFluxBC::computeQpResidual()
{
  Real value;

  if(_t < _duration)
    value = _initial + (_final-_initial) * std::sin((0.5/_duration) * libMesh::pi * _t);
  else
    value = _final;
    
  return -(_phi_face[_i][_qp]*_rate*(value - _u_face[_qp]));
}
  
Real
ConvectiveFluxBC::computeQpJacobian()
{
  Real value;

  if(_t < _duration)
    value = _initial + (_final-_initial) * std::sin((0.5/_duration) * libMesh::pi * _t);
  else
    value = _final;
  
  return -(_phi_face[_i][_qp]*_rate*(-_phi_face[_j][_qp]));
}
