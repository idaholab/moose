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

#include "SideSetsFromNormals.h"
#include "Parser.h"
#include "InputParameters.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"

template <>
InputParameters
validParams<SideSetsFromNormals>()
{
  InputParameters params = validParams<AddSideSetsBase>();
  params.addRequiredParam<std::vector<BoundaryName>>("new_boundary",
                                                     "The name of the boundary to create");
  params.addRequiredParam<std::vector<Point>>(
      "normals", "A list of normals for which to start painting sidesets");
  return params;
}

SideSetsFromNormals::SideSetsFromNormals(const InputParameters & parameters)
  : AddSideSetsBase(parameters), _normals(getParam<std::vector<Point>>("normals"))
{

  // Get the BoundaryIDs from the mesh
  _boundary_names = getParam<std::vector<BoundaryName>>("new_boundary");

  if (_normals.size() != _boundary_names.size())
    mooseError("normal list and boundary list are not the same length");

  // Make sure that the normals are normalized
  for (auto & normal : _normals)
  {
    mooseAssert(normal.norm() >= 1e-5, "Normal is zero");
    normal /= normal.norm();
  }
}

void
SideSetsFromNormals::modify()
{
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling SideSetsFromNormals::modify()!");

  // We can't call this in the constructor, it appears that _mesh_ptr is always NULL there.
  _mesh_ptr->errorIfDistributedMesh("SideSetsFromNormals");

  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(_boundary_names, true);

  setup();

  _visited.clear();

  // We'll need to loop over all of the elements to find ones that match this normal.
  // We can't rely on flood catching them all here...
  MeshBase::const_element_iterator el = _mesh_ptr->getMesh().elements_begin();
  const MeshBase::const_element_iterator end_el = _mesh_ptr->getMesh().elements_end();
  for (; el != end_el; ++el)
  {
    const Elem * elem = *el;

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      if (elem->neighbor(side))
        continue;

      _fe_face->reinit(elem, side);
      const std::vector<Point> & normals = _fe_face->get_normals();

      for (unsigned int i = 0; i < boundary_ids.size(); ++i)
      {
        if (std::abs(1.0 - _normals[i] * normals[0]) < 1e-5)
          flood(*el, _normals[i], boundary_ids[i]);
      }
    }
  }

  finalize();

  BoundaryInfo & boundary_info = _mesh_ptr->getMesh().get_boundary_info();
  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
    boundary_info.sideset_name(boundary_ids[i]) = _boundary_names[i];
}
