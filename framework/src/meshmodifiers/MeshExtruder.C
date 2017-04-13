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
#include "libmesh/elem.h"
#include "libmesh/boundary_info.h"

template <>
InputParameters
validParams<MeshExtruder>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<RealVectorValue>("extrusion_vector",
                                           "The direction and length of the extrusion");
  params.addParam<unsigned int>("num_layers", 1, "The number of layers in the extruded mesh");
  params.addParam<std::vector<BoundaryName>>(
      "bottom_sideset", "The boundary that will be applied to the bottom of the extruded mesh");
  params.addParam<std::vector<BoundaryName>>(
      "top_sideset", "The boundary that will be to the top of the extruded mesh");

  params.addParam<std::vector<SubdomainID>>(
      "existing_subdomains",
      std::vector<SubdomainID>(),
      "The subdomains that will be remapped for specific layers");
  params.addParam<std::vector<unsigned int>>(
      "layers",
      std::vector<unsigned int>(),
      "The layers where the \"existing_subdomain\" will be remapped to new ids");
  params.addParam<std::vector<unsigned int>>(
      "new_ids",
      std::vector<unsigned int>(),
      "The list of new ids, This list should be either length \"existing_subdomains\" or "
      "\"existing_subdomains\" * layers");
  return params;
}

MeshExtruder::MeshExtruder(const InputParameters & parameters)
  : MeshModifier(parameters),
    _extrusion_vector(getParam<RealVectorValue>("extrusion_vector")),
    _num_layers(getParam<unsigned int>("num_layers")),
    _existing_subdomains(getParam<std::vector<SubdomainID>>("existing_subdomains")),
    _layers(getParam<std::vector<unsigned int>>("layers")),
    _new_ids(getParam<std::vector<unsigned int>>("new_ids"))
{
  // Check the length of the vectors
  if (_existing_subdomains.size() != _new_ids.size() &&
      _existing_subdomains.size() * _layers.size() != _new_ids.size())
    mooseError(
        "The length of the \"existing_subdomains\", \"layers\", and \"new_ids\" are not valid");
}

void
MeshExtruder::modify()
{
  // When we clone, we're responsible to clean up after ourselves!
  // TODO: traditionally clone() methods return pointers...
  MooseMesh * source_mesh = &(_mesh_ptr->clone());

  if (source_mesh->getMesh().mesh_dimension() == 3)
    mooseError("You cannot extrude a 3D mesh!");

  _mesh_ptr->getMesh().clear();

  std::unique_ptr<QueryElemSubdomainID> elem_subdomain_id;
  if (_existing_subdomains.size() > 0)
    elem_subdomain_id = libmesh_make_unique<QueryElemSubdomainID>(
        _existing_subdomains, _layers, _new_ids, _num_layers);

  // The first argument to build_extrusion() is required to be UnstructuredMesh&, a common
  // base class of both ReplicatedMesh and DistributedMesh, hence the dynamic_cast...
  MeshTools::Generation::build_extrusion(
      dynamic_cast<libMesh::UnstructuredMesh &>(_mesh_ptr->getMesh()),
      source_mesh->getMesh(),
      _num_layers,
      _extrusion_vector,
      elem_subdomain_id.get());

  // See if the user has requested specific sides for the top and bottom
  std::set<boundary_id_type> side_ids =
      _mesh_ptr->getMesh().get_boundary_info().get_side_boundary_ids();

  // Handle distributed meshes: processors may not know all side ids
  _communicator.set_union(side_ids);

  std::set<boundary_id_type>::reverse_iterator last_side_it = side_ids.rbegin();

  const boundary_id_type old_top = *last_side_it;
  mooseAssert(last_side_it != side_ids.rend(), "Error in generating sidesets for extruded mesh");
  const boundary_id_type old_bottom = *++last_side_it;

  // Update the IDs
  if (isParamValid("bottom_sideset"))
    changeID(getParam<std::vector<BoundaryName>>("bottom_sideset"), old_bottom);
  if (isParamValid("top_sideset"))
    changeID(getParam<std::vector<BoundaryName>>("top_sideset"), old_top);

  // Update the dimension
  _mesh_ptr->getMesh().set_mesh_dimension(source_mesh->getMesh().mesh_dimension() + 1);

  // Clean up the source mesh we allocated
  delete source_mesh;
}

void
MeshExtruder::changeID(const std::vector<BoundaryName> & names, BoundaryID old_id)
{
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(names, true);

  if (std::find(boundary_ids.begin(), boundary_ids.end(), old_id) == boundary_ids.end())
    _mesh_ptr->changeBoundaryId(old_id, boundary_ids[0], true);

  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
  {
    _mesh_ptr->getMesh().get_boundary_info().sideset_name(boundary_ids[i]) = names[i];
    _mesh_ptr->getMesh().get_boundary_info().nodeset_name(boundary_ids[i]) = names[i];
  }
}

MeshExtruder::QueryElemSubdomainID::QueryElemSubdomainID(
    std::vector<SubdomainID> existing_subdomains,
    std::vector<unsigned int> layers,
    std::vector<unsigned int> new_ids,
    unsigned int libmesh_dbg_var(num_layers))
  : QueryElemSubdomainIDBase()
#ifndef NDEBUG
    ,
    _num_layers(num_layers)
#endif
{
  // Setup our stride depending on whether the user passed unique sets in new ids or just a single
  // set of new ids
  const unsigned int zero = 0;
  const unsigned int stride =
      existing_subdomains.size() == new_ids.size() ? zero : existing_subdomains.size();

  // Populate the data structure
  for (unsigned int i = 0; i < layers.size(); ++i)
    for (unsigned int j = 0; j < existing_subdomains.size(); ++j)
      _layer_data[layers[i]][existing_subdomains[j]] = new_ids[i * stride + j];
}

subdomain_id_type
MeshExtruder::QueryElemSubdomainID::get_subdomain_for_layer(const Elem * old_elem,
                                                            unsigned int layer)
{
  mooseAssert(layer < _num_layers, "Access out of bounds: " << layer);

  // First locate the layer if it exists
  std::map<unsigned int, std::map<SubdomainID, unsigned int>>::const_iterator layer_it =
      _layer_data.find(layer);

  if (layer_it == _layer_data.end())
    // If the layer wasn't found, there is no mapping so just return the original subdomain id
    return old_elem->subdomain_id();
  else
  {
    std::map<SubdomainID, unsigned int>::const_iterator sub_id_it =
        layer_it->second.find(old_elem->subdomain_id());

    if (sub_id_it == layer_it->second.end())
      // If the subdomain wasn't found, it won't be remapped, so just return the original subdomain
      // id
      return old_elem->subdomain_id();

    // Return the remapped id
    return sub_id_it->second;
  }
}
