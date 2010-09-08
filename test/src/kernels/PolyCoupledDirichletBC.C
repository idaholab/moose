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

#include "PolyCoupledDirichletBC.h"

template<>
InputParameters validParams<PolyCoupledDirichletBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<bool>("_integrated") = false;
  params.addParam<Real>("value", 0.0, "Value multiplied by the coupled value on the boundary");
  
  return params;
}

PolyCoupledDirichletBC::PolyCoupledDirichletBC(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),

   //Grab the parameter for the multiplier. 
   _value(getParam<Real>("value"))
{}

Real
PolyCoupledDirichletBC::computeQpResidual()
{
  //We define all our variables here along with our function. 
  Real a = libMesh::pi;
  Real b = 3;
  Real e = 4;
  Real x = (*_current_node)(0);
  Real y = (*_current_node)(1);
  Real z = (*_current_node)(2);
  Real t = _t;
  Real u = a*x*x*x*y*t+b*y*y*z+e*x*y*z*z*z*z;
  
  //Our function gets added here. 
  return _u[_qp]-(u);
}
