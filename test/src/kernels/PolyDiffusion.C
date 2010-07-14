#include "PolyDiffusion.h"

template<>
InputParameters validParams<PolyDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

PolyDiffusion::PolyDiffusion(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters)
{}

Real
PolyDiffusion::computeQpResidual()
{
  Real a = libMesh::pi;
  Real b = 3;
  Real e = 4;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);
  Real t = _t;
  Real u = a*x*x*x*y*t+b*y*y*z+e*x*y*z*z*z*z;
  return _grad_test[_i][_qp]*(u*u-2*u+2)*_grad_u[_qp];
  //We multiplied by our k(u).
}

Real
PolyDiffusion::computeQpJacobian()
{
  Real a = libMesh::pi;
  Real b =3;
  Real e =4;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);
  Real t = _t;
  Real u = a*x*x*x*y*t+b*y*y*z+e*x*y*z*z*z;
  return _grad_test[_i][_qp]*(u*u-2*u+2)*_grad_phi[_j][_qp];
  //We multiplied by our k(u)
} 
