#include "MMSDiffusion.h"

template<>
InputParameters validParams<MMSDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}


MMSDiffusion::MMSDiffusion(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters)
{}

Real
MMSDiffusion::computeQpResidual()
{
  Real a = libMesh::pi;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);
  Real t = _t;
  Real u = std::sin(a*x*y*z*t);
  return _grad_test[_i][_qp]*(u*u-2*u+2)*_grad_u[_qp];
  //We multiplied by our k(u).
}

Real
MMSDiffusion::computeQpJacobian()
{
  Real a = libMesh::pi;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);
  Real t = _t;
  Real u = std::sin(a*x*y*z*t);
  return _grad_test[_i][_qp]*(u*u-2*u+2)*_grad_phi[_j][_qp];
  //We multiplied by our k(u)
} 
