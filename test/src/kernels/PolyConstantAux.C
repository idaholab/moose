#include "PolyConstantAux.h"

template<>
InputParameters validParams<PolyConstantAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  return params;
}

PolyConstantAux::PolyConstantAux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :AuxKernel(name, moose_system, parameters)
{}


Real
PolyConstantAux::computeValue()
{
  Real a = libMesh::pi;
  Real b = 3;
  Real e = 4;
  Real x = (*_current_node)(0);
  Real y = (*_current_node)(1);
  Real z = (*_current_node)(2);
  Real t = _t;
  return a*x*x*x*y*t+b*y*y*z+e*x*y*z*z*z*z;

}
