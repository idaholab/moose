//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshExtruderGenerator.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", MeshExtruderGenerator);

InputParameters
MeshExtruderGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "the mesh we want to extrude");
  params.addClassDescription("Takes a 1D or 2D mesh and extrudes the entire structure along the "
                             "specified axis increasing the dimensionality of the mesh.");
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

MeshExtruderGenerator::MeshExtruderGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
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

std::unique_ptr<MeshBase>
MeshExtruderGenerator::generate()
{
  std::unique_ptr<MeshBase> source_mesh = std::move(_input);

  auto dest_mesh = buildMeshBaseObject();

  // check that the existing_subdomains exist in the mesh
  for (const auto & id : _existing_subdomains)
    if (!MooseMeshUtils::hasSubdomainID(*source_mesh, id))
      paramError("existing_subdomains", "The block ID '", id, "' was not found in the mesh");

  if (source_mesh->mesh_dimension() == 3)
    mooseError("You cannot extrude a 3D mesh !");

  std::unique_ptr<QueryElemSubdomainID> elem_subdomain_id;
  if (_existing_subdomains.size() > 0)
    elem_subdomain_id = std::make_unique<QueryElemSubdomainID>(
        _existing_subdomains, _layers, _new_ids, _num_layers);

  MeshTools::Generation::build_extrusion(dynamic_cast<libMesh::UnstructuredMesh &>(*dest_mesh),
                                         *source_mesh,
                                         _num_layers,
                                         _extrusion_vector,
                                         elem_subdomain_id.get());

  // See if the user has requested specific sides for the top and bottom
  std::set<boundary_id_type> side_ids = dest_mesh->get_boundary_info().get_side_boundary_ids();

  // Handle distributed meshes: processors may not know all side ids
  _communicator.set_union(side_ids);

  std::set<boundary_id_type>::reverse_iterator last_side_it = side_ids.rbegin();

  const boundary_id_type old_top = *last_side_it;
  mooseAssert(last_side_it != side_ids.rend(), "Error in generating sidesets for extruded mesh");
  const boundary_id_type old_bottom = *++last_side_it;

  // Update the IDs
  if (isParamValid("bottom_sideset"))
    changeID(*dest_mesh, getParam<std::vector<BoundaryName>>("bottom_sideset"), old_bottom);
  if (isParamValid("top_sideset"))
    changeID(*dest_mesh, getParam<std::vector<BoundaryName>>("top_sideset"), old_top);

  return dynamic_pointer_cast<MeshBase>(dest_mesh);
}

void
MeshExtruderGenerator::changeID(MeshBase & mesh,
                                const std::vector<BoundaryName> & names,
                                BoundaryID old_id)
{
  std::vector<boundary_id_type> boundary_ids = MooseMeshUtils::getBoundaryIDs(mesh, names, true);

  if (std::find(boundary_ids.begin(), boundary_ids.end(), old_id) == boundary_ids.end())
    MooseMeshUtils::changeBoundaryId(mesh, old_id, boundary_ids[0], true);

  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
  {
    mesh.get_boundary_info().sideset_name(boundary_ids[i]) = names[i];
    mesh.get_boundary_info().nodeset_name(boundary_ids[i]) = names[i];
  }
}

MeshExtruderGenerator::QueryElemSubdomainID::QueryElemSubdomainID(
    const std::vector<SubdomainID> & existing_subdomains,
    std::vector<unsigned int> layers,
    const std::vector<unsigned int> & new_ids,
    unsigned int num_layers)
  : QueryElemSubdomainIDBase(), _num_layers(num_layers)
{
  // Setup our stride depending on whether the user passed unique sets in new ids or just a single
  // set of new ids
  const unsigned int stride =
      existing_subdomains.size() == new_ids.size() ? 0 : existing_subdomains.size();

  if (layers.size() == 0)
    for (unsigned int i = 0; i < _num_layers; i++)
      layers.push_back(i);

  // Populate the data structure
  for (unsigned int i = 0; i < layers.size(); ++i)
    for (unsigned int j = 0; j < existing_subdomains.size(); ++j)
      _layer_data[layers[i]][existing_subdomains[j]] = new_ids[i * stride + j];
}

subdomain_id_type
MeshExtruderGenerator::QueryElemSubdomainID::get_subdomain_for_layer(const Elem * old_elem,
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
