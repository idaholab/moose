#include "MMSConstantAux.h"

template<>
InputParameters validParams<MMSConstantAux>()
{
  InputParameters params = validParams<AuxKernel>();

  return params;
}

MMSConstantAux::MMSConstantAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _mesh_dimension(_mesh.dimension())
{}


Real
MMSConstantAux::computeValue()
{
  Real a = libMesh::pi;
  Real x = (*_current_node)(0);
  Real y = (*_current_node)(1);
  Real t = _t;

  if (_mesh_dimension == 3)
  {
    Real z = (*_current_node)(2);
    return std::sin(a*x*y*z*t);
  }
  else
  {
    Real z = 1.0;
    return std::sin(a*x*y*z*t);
  }
}
