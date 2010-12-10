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

#include "SlaveConstraint.h"

// Moose includes
#include "MooseSystem.h"

// libmesh includes
#include "plane.h"

template<>
InputParameters validParams<SlaveConstraint>()
{
  InputParameters params = validParams<DiracKernel>();
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

SlaveConstraint::SlaveConstraint(const std::string & name, InputParameters parameters)
  :DiracKernel(name, parameters),
   _residual_copy(residualCopy())
{
  _node = _mesh.node_ptr(4);
}
           
void
SlaveConstraint::addPoints()
{
  addPoint(*_node);
}

Real
SlaveConstraint::computeQpResidual()
{
  Plane plane(Point(0.5,1.0), Point(0,2.0), Point(0,2.0,-1.0));
  
  return _phi[_i][_qp]*(plane.closest_point(*_node)-(*_node)).size();
}

