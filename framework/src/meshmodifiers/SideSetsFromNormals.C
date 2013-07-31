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

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"

template<>
InputParameters validParams<SideSetsFromNormals>()
{
  InputParameters params = validParams<AddSideSetsBase>();
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "A list of boundary names to associate with the painted sidesets");
  params.addRequiredParam<std::vector<Point> >("normals", "A list of normals for which to start painting sidesets");
  return params;
}

SideSetsFromNormals::SideSetsFromNormals(const std::string & name, InputParameters parameters):
    AddSideSetsBase(name, parameters),
    _normals(getParam<std::vector<Point> >("normals")),
    _boundary_names(getParam<std::vector<BoundaryName> >("boundary"))
{
  if (_normals.size() != _boundary_names.size())
    mooseError("normal list and boundary list are not the same length");

  // Make sure that the normals are normalized
  for (std::vector<Point>::iterator normal_it = _normals.begin(); normal_it != _normals.end(); ++normal_it)
  {
    mooseAssert(normal_it->size() >= 1e-5, "Normal is zero");
    *normal_it /= normal_it->size();
  }
}

SideSetsFromNormals::~SideSetsFromNormals()
{
}

void
SideSetsFromNormals::modify()
{
  // Get the BoundaryIDs from the mesh
  _boundary_ids = _mesh_ptr->getBoundaryIDs(_boundary_names, true);

  setup();

  _visited.clear();

  // We'll need to loop over all of the elements to find ones that match this normal.
  // We can't rely on flood catching them all here...
  MeshBase::const_element_iterator       el     = _mesh_ptr->getMesh().elements_begin();
  const MeshBase::const_element_iterator end_el = _mesh_ptr->getMesh().elements_end();
  for ( ; el != end_el ; ++el)
  {
    const Elem *elem = *el;

    for (unsigned int side=0; side < elem->n_sides(); ++side)
    {
      if (elem->neighbor(side))
        continue;

      _fe_face->reinit(elem, side);
      const std::vector<Point> & normals = _fe_face->get_normals();

      for (unsigned int i=0; i<_boundary_ids.size(); ++i)
      {
        if (std::abs(1.0 - _normals[i]*normals[0]) < 1e-5)
          flood(*el, _normals[i], _boundary_ids[i]);
      }
    }
  }

  finalize();

  for (unsigned int i=0; i<_boundary_ids.size(); ++i)
    _mesh_ptr->getMesh().boundary_info->sideset_name(_boundary_ids[i]) = _boundary_names[i];

}
