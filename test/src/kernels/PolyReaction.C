#include "PolyReaction.h"

template<>
InputParameters validParams<PolyReaction>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

PolyReaction::PolyReaction(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters)
{}

Real
PolyReaction::computeQpResidual()
{
  Real a = libMesh::pi;
  Real b = 3;
  Real e = 4;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);
  Real t = _t;
  Real u = a*x*x*x*y*t+b*y*y*z+e*x*y*z*z*z*z;
  return _test[_i][_qp]*2*u*u;
}

Real
PolyReaction::computeQpJacobian()
{
  return 0; //We have no grad u.
}
