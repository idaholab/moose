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

#include "MMSConstantAux.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<MMSConstantAux>()
{
  InputParameters params = validParams<AuxKernel>();

  return params;
}

MMSConstantAux::MMSConstantAux(const InputParameters & parameters)
  : AuxKernel(parameters), _mesh_dimension(_mesh.dimension())
{
}

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
    return std::sin(a * x * y * z * t);
  }
  else
  {
    Real z = 1.0;
    return std::sin(a * x * y * z * t);
  }
}
