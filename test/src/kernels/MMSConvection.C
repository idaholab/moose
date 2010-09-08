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

MMSConvection::MMSConvection(const std::string & name,
                       MooseSystem &sys,
                       InputParameters parameters)
 
  :Kernel(name, sys, parameters),
   
   _x(getParam<Real>("x")),
   _y(getParam<Real>("y")),
   _z(getParam<Real>("z"))
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
  Real t = _t;

  if (_dim == 3)
  {
    Real z = _q_point[_qp](2);
    Real u = sin(a*x*y*z*t);
    return _test[_i][_qp]*(velocity*_grad_u[_qp]);
  }
  else
  {
    Real z = 1;
    Real u = sin(a*x*y*z*t);
    return _test[_i][_qp]*(velocity*_grad_u[_qp]);
  }
}

Real MMSConvection::computeQpJacobian()
{
  //There is no Jacobian since we have no grad u.
  return 0;
}
