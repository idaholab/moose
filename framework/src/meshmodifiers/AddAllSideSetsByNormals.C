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

#include "AddAllSideSetsByNormals.h"
#include "Parser.h"
#include "InputParameters.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"

template<>
InputParameters validParams<AddAllSideSetsByNormals>()
{
  InputParameters params = validParams<AddSideSetsBase>();
  return params;
}

AddAllSideSetsByNormals::AddAllSideSetsByNormals(const std::string & name, InputParameters parameters):
    AddSideSetsBase(name, parameters)
{
}

AddAllSideSetsByNormals::~AddAllSideSetsByNormals()
{
}

void
AddAllSideSetsByNormals::modify()
{
  setup();

  // Get the current list of boundaries so we can generate new ones that won't conflict
  _mesh_boundary_ids = &_mesh_ptr->_mesh_boundary_ids;

  // Create the map object that will be owned by MooseMesh
  AutoPtr<std::map<BoundaryID, RealVectorValue> > boundary_map(new std::map<BoundaryID, RealVectorValue>());

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

      {
        // See if we've seen this normal before (linear search)
        std::map<BoundaryID, RealVectorValue>::iterator it = boundary_map->begin();
        while (it != boundary_map->end())
        {
          if (std::abs(1.0 - it->second*normals[0]) < 1e-5)
            break;
          ++it;
        }

        if (it != boundary_map->end())  // Found it!
          flood(*el, normals[0], it->first);
        else
        {
          BoundaryID id = getNextBoundaryID();
          (*boundary_map)[id] = normals[0];
          flood(*el, normals[0], id);
        }
      }
    }
  }

  finalize();

  // Transfer owndership of the boundary map.
  _mesh_ptr->_boundary_to_normal_map = boundary_map;
}

BoundaryID
AddAllSideSetsByNormals::getNextBoundaryID()
{
  std::set<BoundaryID>::iterator it;
  BoundaryID next_id = 1;

  while ((it = _mesh_boundary_ids->find(next_id)) != _mesh_boundary_ids->end())
    ++next_id;

  _mesh_boundary_ids->insert(next_id);

  return next_id;
}
