#include "MMSConvection.h"

template<>
InputParameters validParams<MMSConvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("x", "Component of velocity in the x direction");
  params.addRequiredParam<Real>("y", "Component of velocity in the y direction");
  params.addParam<Real>("z", 0.0, "Component of velocity in the z direction");
  return params;
}

MMSConvection::MMSConvection(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters)
 
  :Kernel(name, sys, parameters),
   
   _x(_parameters.get<Real>("x")),
   _y(_parameters.get<Real>("y")),
   _z(_parameters.get<Real>("z"))
{
  
  velocity(0)=_x;
  velocity(1)=_y;
  velocity(2)=_z;  
}

Real MMSConvection::computeQpResidual()
{
  // We changed Residual because we have no grad u.
  Real a = libMesh::pi;
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  Real z = _q_point[_qp](2);
  Real t = _t;
  Real u = sin(a*x*y*z*t);  
  return _test[_i][_qp]*(velocity*_grad_u[_qp]);
}

Real MMSConvection::computeQpJacobian()
{
  //There is no Jacobian since we have no grad u.
  return 0;
}
