#include "MMSCoupledDirichletBC.h"

template<>
InputParameters validParams<MMSCoupledDirichletBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addParam<Real>("value", 0.0, "Value multiplied by the coupled value on the boundary");
  
  return params;
}

MMSCoupledDirichletBC::MMSCoupledDirichletBC(const std::string & name, InputParameters parameters) :
    NodalBC(name, parameters),
    //Grab the parameter for the multiplier.
    _value(getParam<Real>("value"))
{}

Real
MMSCoupledDirichletBC::computeQpResidual()
{
  //We define all our variables here along with our function. 
  Real a = libMesh::pi;
  Real x = (*_current_node)(0);
  Real y = (*_current_node)(1);
  Real t = _t;
  if (_dim == 3)
  {
    Real z = (*_current_node)(2);
    Real u = sin(a*x*y*z*t);
    //Our function gets added here.
    return _u[_qp]-u;
  }
  else
  {
    Real z = 1.0;
    Real u = sin(a*x*y*z*t);
    //Our function gets added here.
    return _u[_qp]-u;
  }
  
}
