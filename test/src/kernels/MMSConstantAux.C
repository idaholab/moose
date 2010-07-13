#include "MMSConstantAux.h"

template<>
InputParameters validParams<MMSConstantAux>()
{
  InputParameters params = validParams<AuxKernel>();
  
  return params;
}

MMSConstantAux::MMSConstantAux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :AuxKernel(name, moose_system, parameters)
{}


Real
MMSConstantAux::computeValue()
{
  Real a = libMesh::pi;
  Real x = (*_current_node)(0);
  Real y = (*_current_node)(1);
  Real z = (*_current_node)(2);
  Real t = _t;
  return std::sin(a*x*y*z*t);

}
