#include "ConvectiveFluxBC.h"

template<>
Parameters valid_params<ConvectiveFluxBC>()
{
  Parameters params;
  params.set<Real>("rate")=7500;
  params.set<Real>("initial")=500;
  params.set<Real>("final")=500;
  params.set<Real>("duration")=0.0;
  return params;
}

Real ConvectiveFluxBC::computeQpResidual()
{
  Real value;

  if(_t < _duration)
    value = _initial + (_final-_initial) * std::sin((0.5/_duration) * libMesh::pi * _t);
  else
    value = _final;
    
  return -(_phi_face[_i][_qp]*_rate*(value - _u_face[_qp]));
}
  
Real ConvectiveFluxBC::computeQpJacobian()
{
  Real value;

  if(_t < _duration)
    value = _initial + (_final-_initial) * std::sin((0.5/_duration) * libMesh::pi * _t);
  else
    value = _final;
  
  return -(_phi_face[_i][_qp]*_rate*(-_phi_face[_j][_qp]));
}
