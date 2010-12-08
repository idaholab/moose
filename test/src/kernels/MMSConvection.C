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
                       InputParameters parameters)
 
  :Kernel(name, parameters),
   
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
  return _test[_i][_qp]*(velocity*_grad_u[_qp]);
}

Real MMSConvection::computeQpJacobian()
{
  //There is no Jacobian since we have no grad u.
  return 0;
}
