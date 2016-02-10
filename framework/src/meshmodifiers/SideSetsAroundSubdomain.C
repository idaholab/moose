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

#include "SideSetsAroundSubdomain.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/mesh.h"

template<>
InputParameters validParams<SideSetsAroundSubdomain>()
{
  InputParameters params = validParams<AddSideSetsBase>();
  params += validParams<BlockRestrictable>();
  params.addRequiredParam<std::vector<BoundaryName> >("new_boundary", "The list of boundary IDs to create on the supplied subdomain");
  params.addParam<Point>("normal", "If supplied, only faces with normal equal to this, up to normal_tol, will be added to the sidesets specified");
  params.addRangeCheckedParam<Real>("normal_tol", 0.1, "normal_tol>=0 & normal_tol<=2", "If normal is supplied then faces are only added if face_normal.normal_hat >= 1 - normal_tol, where normal_hat = normal/|normal|");
  params.addClassDescription("Adds element faces that are on the exterior of the given block to the sidesets specified");
  return params;
}

SideSetsAroundSubdomain::SideSetsAroundSubdomain(const InputParameters & parameters):
    AddSideSetsBase(parameters),
    BlockRestrictable(parameters),
    _boundary_names(getParam<std::vector<BoundaryName> >("new_boundary")),
    _using_normal(isParamValid("normal")),
    _normal_tol(getParam<Real>("normal_tol")),
    _normal(_using_normal ? getParam<Point>("normal") : Point())
{

  if (_using_normal)
  {
    // normalize
    mooseAssert(_normal.norm() >= 1E-5, "Normal is zero");
    _normal /= _normal.norm();
  }
}

SideSetsAroundSubdomain::~SideSetsAroundSubdomain()
{
}

void
SideSetsAroundSubdomain::modify()
{
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling SideSetsAroundSubdomain::modify()!");

  // Reference the the libMesh::MeshBase
  MeshBase & mesh = _mesh_ptr->getMesh();

  // Extract the 'first' block ID
  SubdomainID block_id = *blockIDs().begin();

  // Extract the SubdomainID
  if (numBlocks() > 1)
     mooseWarning("SideSetsAroundSubdomain only acts on a single subdomain, but multiple were provided: only the " << block_id << "' subdomain is being used.");

  // Create the boundary IDs from the list of names provided (the true flag creates ids from unknown names)
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(_boundary_names, true);

  // construct the FE object so we can compute normals of faces
  setup();
  Point face_normal;
  bool add_to_bdy = true;

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Loop over the elements
  MeshBase::const_element_iterator   el  = mesh.active_elements_begin();
  const MeshBase::const_element_iterator end_el = mesh.active_elements_end();
  for (; el != end_el ; ++el)
  {
    const Elem* elem = *el;
    SubdomainID curr_subdomain = elem->subdomain_id();

    // We only need to loop over elements in the source subdomain
    if (curr_subdomain != block_id)
      continue;

    for (unsigned int side = 0; side < elem->n_sides(); side++)
    {
      const Elem * neighbor = elem->neighbor(side);
      if (neighbor == NULL ||                   // element on boundary OR
          neighbor->subdomain_id() != block_id) // neighboring element is on a different subdomain
        {
          if (_using_normal)
          {
            _fe_face->reinit(elem, side);
            face_normal = _fe_face->get_normals()[0];
            add_to_bdy = (_normal*face_normal >= 1.0 - _normal_tol);
          }


          // Add the boundaries, if appropriate
          if (add_to_bdy)
            for (unsigned int i = 0; i < boundary_ids.size(); ++i)
              boundary_info.add_side(elem, side, boundary_ids[i]);
        }
    }
  }

  finalize();

  // Assign the supplied names to the newly created side sets
  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
    boundary_info.sideset_name(boundary_ids[i]) = _boundary_names[i];
}

