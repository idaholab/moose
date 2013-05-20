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

#include "MeshExtruder.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/boundary_info.h"

template<>
InputParameters validParams<MeshExtruder>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<unsigned int>("num_layers", "The number of layers in the extruded mesh");
  params.addRequiredParam<RealVectorValue>("extrusion_vector", "The direction and length of the extrusion");
  params.addParam<std::vector<BoundaryName> >("bottom_sideset", "The boundary that will be applied to the bottom of the extruded mesh");
  params.addParam<std::vector<BoundaryName> >("top_sideset", "The boundary that will be to the top of the extruded mesh");
  return params;
}

MeshExtruder::MeshExtruder(const std::string & name, InputParameters parameters):
    MeshModifier(name, parameters),
    _num_layers(getParam<unsigned int>("num_layers")),
    _extrusion_vector(getParam<RealVectorValue>("extrusion_vector"))
{
}

MeshExtruder::~MeshExtruder()
{
}

void
MeshExtruder::modify()
{
  Mesh source_mesh(_mesh_ptr->_mesh);  // copy constructor
  if (source_mesh.mesh_dimension() == 3)
    mooseError("You cannot extrude a 3D mesh!");

  _mesh_ptr->_mesh.clear();

  MeshTools::Generation::build_extrusion(_mesh_ptr->_mesh, source_mesh, _num_layers, _extrusion_vector);

  // See if the user has requested specific sides for the top and bottom
  const std::set<boundary_id_type> &side_ids = _mesh_ptr->_mesh.boundary_info->get_side_boundary_ids();
  std::set<boundary_id_type>::reverse_iterator last_side_it = side_ids.rbegin();

  const boundary_id_type old_top = *last_side_it;
  mooseAssert(last_side_it != side_ids.rend(), "Error in generating sidesets for extruded mesh");
  const boundary_id_type old_bottom = *++last_side_it;

  // Update the IDs
  if (isParamValid("bottom_sideset"))
    changeID(getParam<std::vector<BoundaryName> >("bottom_sideset"), old_bottom);
  if (isParamValid("top_sideset"))
    changeID(getParam<std::vector<BoundaryName> >("top_sideset"), old_top);

  // Update the dimension
  _mesh_ptr->_mesh.set_mesh_dimension(source_mesh.mesh_dimension() + 1);
}


void
MeshExtruder::changeID(const std::vector<BoundaryName> & names, BoundaryID old_id)
{
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(names, true);

  if (std::find(boundary_ids.begin(), boundary_ids.end(), old_id) == boundary_ids.end())
    _mesh_ptr->changeBoundaryId(old_id, boundary_ids[0], true);

  for (unsigned int i=0; i<boundary_ids.size(); ++i)
    _mesh_ptr->_mesh.boundary_info->sideset_name(boundary_ids[i]) = names[i];
}


