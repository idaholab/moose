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

#include "BoundingBoxNodeSet.h"

template<>
InputParameters validParams<BoundingBoxNodeSet>()
{
  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<std::vector<BoundaryName> >("new_boundary", "The name of the nodeset to create");
  params.addParam<RealVectorValue>("bottom_left", "The bottom left point (in x,y,z with spaces in-between) of the box which contains the centroids of the elements whose nodes will be selected.");
  params.addParam<RealVectorValue>("top_right", "The bottom left point (in x,y,z with spaces in-between) of the box which contains the centroids of the elements whose nodes will be selected.");
  params.addParam<MooseEnum>("location", location, "Control of where the nodeset is to be set");

  return params;
}

BoundingBoxNodeSet::BoundingBoxNodeSet(const std::string & name, InputParameters params) :
    MeshModifier(name, params),
    _location(params.get<MooseEnum>("location")),
    _bounding_box(params.get<RealVectorValue>("bottom_left"), params.get<RealVectorValue>("top_right"))
{
}

void
BoundingBoxNodeSet::modify()
{

  // Get the BoundaryIDs from the mesh
  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName> >("new_boundary");
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(boundary_names, true);
  // Get a reference to our BoundaryInfo object
  BoundaryInfo & boundary_info = _mesh_ptr->getMesh().get_boundary_info();

  if (_pars.isParamValid("bottom_left") && _pars.isParamValid("top_right"))
  {
    int num_bound_ids = boundary_ids.size();
    if (num_bound_ids != 1)//you cannot define more than one nodeset within a bounding box.
      mooseError("Only one boundary ID can be assigned to a nodeset using a bounding box!");

    // Check that we have access to the mesh
    if (!_mesh_ptr)
      mooseError("_mesh_ptr must be initialized before calling BoundingBoxNodeSet::modify()");

    // Reference the the libMesh::MeshBase
    MeshBase & mesh = _mesh_ptr->getMesh();
    bool found_node = false;
    // Loop over the elements and assign node set id to nodes within the bounding box
    for (MeshBase::element_iterator el = mesh.active_elements_begin(); el != mesh.active_elements_end(); ++el)
    {
      const Elem* elem = *el;

      bool contains = _bounding_box.contains_point((*el)->centroid());
      if ((contains && _location == "INSIDE") || (!contains && _location == "OUTSIDE"))
      {
        for (unsigned int j=0; j<elem->n_nodes(); j++)
        {
          const Node* node = elem->get_node(j);

          for (unsigned int j=0; j<boundary_ids.size(); ++j)
              boundary_info.add_node(node, boundary_ids[j]);

          found_node = true;
        }
      }
    }
    if (!found_node)
      mooseError("No element centroids found within the bounding box!");
  }
  else
    mooseError("Node set can not be empty!");

  for (unsigned int i=0; i<boundary_ids.size(); ++i)
    boundary_info.nodeset_name(boundary_ids[i]) = boundary_names[i];
}
