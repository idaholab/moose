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

#include "SideSetsFromPoints.h"
#include "Parser.h"
#include "InputParameters.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"

template<>
InputParameters validParams<SideSetsFromPoints>()
{
  InputParameters params = validParams<AddSideSetsBase>();
  params += validParams<BoundaryRestrictableRequired>();
  params.addRequiredParam<std::vector<Point> >("points", "A list of points from which to start painting sidesets");
  return params;
}

SideSetsFromPoints::SideSetsFromPoints(const std::string & name, InputParameters parameters):
    AddSideSetsBase(name, parameters),
    BoundaryRestrictableRequired(name, parameters),
    _boundary_ids(boundaryIDs().begin(), boundaryIDs().end()),
    _points(getParam<std::vector<Point> >("points"))
{
  if (_points.size() != boundaryNames().size())
    mooseError("point list and boundary list are not the same length");
}

SideSetsFromPoints::~SideSetsFromPoints()
{
}

void
SideSetsFromPoints::modify()
{
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling SideSetsFromPoints::modify()!");

  // We can't call this in the constructor, it appears that _mesh_ptr is always NULL there.
  _mesh_ptr->errorIfParallelDistribution("SideSetsFromPoints");

  // Get the BoundaryIDs from the mesh
  _boundary_ids = _mesh_ptr->getBoundaryIDs(boundaryNames(), true);

  setup();

  _visited.clear();

  AutoPtr<PointLocatorBase> pl = PointLocatorBase::build(TREE, *_mesh_ptr);

  for (unsigned int i=0; i< _boundary_ids.size(); ++i)
  {
    const Elem * elem = (*pl)(_points[i]);

    for (unsigned int side=0; side < elem->n_sides(); ++side)
    {
      if (elem->neighbor(side))
        continue;

      // See if this point is on this side
      AutoPtr<Elem> elem_side = elem->side(side);

      if (elem_side->contains_point(_points[i]))
      {
        // This is the side that we want to paint our sideset with
        // First get the normal
        _fe_face->reinit(elem, side);
        const std::vector<Point> & normals = _fe_face->get_normals();

        flood(elem, normals[0], _boundary_ids[i]);
      }
    }
  }

  finalize();

  for (unsigned int i=0; i<_boundary_ids.size(); ++i)
    _mesh_ptr->getMesh().boundary_info->sideset_name(_boundary_ids[i]) = boundaryNames()[i];
}
