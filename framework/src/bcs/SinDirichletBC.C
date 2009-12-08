#include "SinDirichletBC.h"
 
template<>
InputParameters validParams<SinDirichletBC>()
{
  InputParameters params;
  params.set<Real>("initial")=0.0;
  params.set<Real>("final")=0.0;
  params.set<Real>("duration")=0.0;
  return params;
}

SinDirichletBC::SinDirichletBC(std::string name,
		  InputParameters parameters, 
		  std::string var_name, unsigned int boundary_id, 
		  std::vector<std::string> coupled_to, 
		  std::vector<std::string> coupled_as)
  :BoundaryCondition(name, parameters, var_name, false, boundary_id, coupled_to, coupled_as),
   _initial(_parameters.get<Real>("initial")),
   _final(_parameters.get<Real>("final")),
   _duration(_parameters.get<Real>("duration"))
  {}

Real
SinDirichletBC::computeQpResidual()
  {
  
    Real value;

    if(_t < _duration)
      value = _initial + (_final-_initial) * std::sin((0.5/_duration) * libMesh::pi * _t);
    else
      value = _final;
    
    return _u_face[_qp]- value;
  }
