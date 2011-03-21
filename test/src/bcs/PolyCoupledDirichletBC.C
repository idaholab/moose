#include "PolyCoupledDirichletBC.h"

template<>
InputParameters validParams<PolyCoupledDirichletBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<bool>("_integrated") = false;
  params.addParam<Real>("value", 0.0, "Value multiplied by the coupled value on the boundary");
  
  return params;
}

PolyCoupledDirichletBC::PolyCoupledDirichletBC(const std::string & name, InputParameters parameters) :
    NodalBC(name, parameters),
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
