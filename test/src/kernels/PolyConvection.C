/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "PolyConvection.h"

template<>
InputParameters validParams<PolyConvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("x", "Component of velocity in the x direction");
  params.addRequiredParam<Real>("y", "Component of velocity in the y direction");
  params.addParam<Real>("z", 0.0, "Component of velocity in the z direction");
  return params;
}

PolyConvection::PolyConvection(const std::string & name,
                       InputParameters parameters)
 
  :Kernel(name, parameters),

   // This is the "Intialization List" it sets the values of class variables
   _x(getParam<Real>("x")),
   _y(getParam<Real>("y")),
   _z(getParam<Real>("z"))
{
  
  velocity(0)=_x;
  velocity(1)=_y;
  velocity(2)=_z;  
}

Real PolyConvection::computeQpResidual()
{
  // We changed Residual because we have no grad u.
//  Real a = libMesh::pi;
//  Real b = 3;
//  Real e = 4;
//  Real x = _q_point[_qp](0);
//  Real y = _q_point[_qp](1);
//  Real z = _q_point[_qp](2);
//  Real t = _t;
//  Real u = a*x*x*x*y*t+b*y*y*z+e*x*y*z*z*z*z;
  return _test[_i][_qp]*(velocity*_grad_u[_qp]);
}

Real PolyConvection::computeQpJacobian()
{
  //There is no Jacobian since we have no grad u.
  return 0;
}
