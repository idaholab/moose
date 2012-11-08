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

#include "PeriodicDistanceAux.h"
#include "GeneratedMesh.h"

template<>
InputParameters validParams<PeriodicDistanceAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<Point>("point", "Some point in the domain");
  return params;
}

PeriodicDistanceAux::PeriodicDistanceAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _nl(static_cast<FEProblem &>(_subproblem).getNonlinearSystem()),
    _point(getParam<Point>("point")),
    _gen_mesh(dynamic_cast<GeneratedMesh *>(&_mesh))
{
  // We aren't going to couple to anything so just use the first nl variable
  _gen_mesh->initPeriodicDistanceForVariable(_nl, 0);

  // Make sure the point is in the domain
  for (unsigned int i=0; i<LIBMESH_DIM; ++i)
    if (_point(i) < _gen_mesh->getMinInDimension(i) || _point(i) > _gen_mesh->getMaxInDimension(i))
      mooseError("\"point\" is outside of the domain.");
}

PeriodicDistanceAux::~PeriodicDistanceAux()
{
}

Real
PeriodicDistanceAux::computeValue()
{
  // Compute the periodic distance from a given feature
  return _gen_mesh->minPeriodicDistance(*_current_node, _point);
}
