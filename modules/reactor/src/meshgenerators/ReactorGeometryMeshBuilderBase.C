//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactorGeometryMeshBuilderBase.h"

InputParameters
ReactorGeometryMeshBuilderBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addParam<bool>(
      "show_rgmb_metadata", false, "Print out RGMB-related metadata to console output");
  params.addClassDescription("A base class that contains common members and methods for Reactor "
                             "Geometry Mesh Builder mesh generators.");

  return params;
}

ReactorGeometryMeshBuilderBase::ReactorGeometryMeshBuilderBase(const InputParameters & parameters)
  : MeshGenerator(parameters)
{
}

void
ReactorGeometryMeshBuilderBase::initializeReactorMeshParams(const std::string reactor_param_name)
{
  _reactor_params = reactor_param_name;

  // Ensure that the user has supplied a valid ReactorMeshParams object
  _reactor_params_mesh = &getMeshByName(reactor_param_name);
  if (*_reactor_params_mesh)
    mooseError("The reactor_params mesh is not of the correct type");

  if (!hasMeshProperty<int>("mesh_dimensions", _reactor_params) ||
      !hasMeshProperty<std::string>("mesh_geometry", _reactor_params))
    mooseError("The reactor_params input must be a ReactorMeshParams type MeshGenerator\n Please "
               "check that a valid definition and name of ReactorMeshParams has been provided.");

  // Set reactor_params_name metadata for use by future mesh generators
  declareMeshProperty("reactor_params_name", std::string(_reactor_params));
}

void
ReactorGeometryMeshBuilderBase::freeReactorMeshParams()
{
  _reactor_params_mesh->reset();
}

unsigned int
ReactorGeometryMeshBuilderBase::getElemIntegerFromMesh(MeshBase & input_mesh,
                                                       std::string extra_int_name,
                                                       bool should_exist)
{
  if (input_mesh.has_elem_integer(extra_int_name))
    return input_mesh.get_elem_integer_index(extra_int_name);
  else
  {
    if (should_exist)
      mooseError("Expected extruded mesh to have " + extra_int_name + " extra integers");
    else
      return input_mesh.add_elem_integer(extra_int_name);
  }
}

void
ReactorGeometryMeshBuilderBase::updateElementBlockNameId(
    MeshBase & input_mesh,
    Elem * elem,
    std::map<std::string, SubdomainID> & name_id_map,
    std::string elem_block_name,
    SubdomainID & next_free_id)
{
  SubdomainID elem_block_id;
  if (name_id_map.find(elem_block_name) == name_id_map.end())
  {
    // Block name does not exist in mesh yet, assign new block id and name
    elem_block_id = next_free_id++;
    elem->subdomain_id() = elem_block_id;
    input_mesh.subdomain_name(elem_block_id) = elem_block_name;
    name_id_map[elem_block_name] = elem_block_id;
  }
  else
  {
    // Block name exists in mesh, reuse block id
    elem_block_id = name_id_map[elem_block_name];
    elem->subdomain_id() = elem_block_id;
  }
}

void
ReactorGeometryMeshBuilderBase::printReactorMetadata(const std::string geometry_type,
                                                     const std::string mg_name,
                                                     bool first_function_call)
{
  if (first_function_call)
  {
    _console << "Global metadata defined using Reactor Geometry Mesh Builder:" << std::endl;
    printGlobalReactorMetadata();
  }
  if (geometry_type == "core")
  {
    _console << "Core-level metadata defined using Reactor Geometry Mesh Builder for " << mg_name
             << ":" << std::endl;
    printCoreMetadata(mg_name, first_function_call);
  }
  else if (geometry_type == "assembly")
  {
    _console << "Assembly-level metadata defined using Reactor Geometry Mesh Builder for "
             << mg_name << ":" << std::endl;
    printAssemblyMetadata(mg_name, first_function_call);
  }
  else if (geometry_type == "pin")
  {
    _console << "Pin-level metadata defined using Reactor Geometry Mesh Builder for " << mg_name
             << ":" << std::endl;
    printPinMetadata(mg_name);
  }
  if (first_function_call)
    _console << std::endl;
}

void
ReactorGeometryMeshBuilderBase::printCoreMetadata(const std::string mg_name,
                                                  bool first_function_call)
{
  bool has_mesh_periphery = hasMeshProperty<Real>(RGMB::peripheral_ring_radius, mg_name);
  if (has_mesh_periphery)
  {
    printMetadataToConsole<Real>(RGMB::peripheral_ring_radius, mg_name);
    printMetadataToConsole<subdomain_id_type>(RGMB::peripheral_ring_region_id, mg_name);
  }

  printMetadataToConsole<std::vector<std::string>>(RGMB::assembly_names, mg_name);
  print2dMetadataToConsole<int>(RGMB::assembly_lattice, mg_name);

  if (first_function_call)
  {
    const auto core_assembly_names =
        getMeshProperty<std::vector<std::string>>(RGMB::assembly_names, mg_name);
    for (const auto & assembly_name : core_assembly_names)
      printReactorMetadata("assembly", assembly_name, false);

    const auto core_pin_names = getMeshProperty<std::vector<std::string>>(RGMB::pin_names, mg_name);
    for (const auto & pin_name : core_pin_names)
      printReactorMetadata("pin", pin_name, false);
  }
}

void
ReactorGeometryMeshBuilderBase::printAssemblyMetadata(const std::string mg_name,
                                                      bool first_function_call)
{
  printMetadataToConsole<subdomain_id_type>(RGMB::assembly_type, mg_name);
  printMetadataToConsole<Real>(RGMB::pitch, mg_name);
  printMetadataToConsole<bool>(RGMB::is_homogenized, mg_name);
  printMetadataToConsole<bool>(RGMB::is_single_pin, mg_name);
  printMetadataToConsole<std::vector<subdomain_id_type>>(RGMB::background_region_id, mg_name);

  const auto duct_halfpitch = getMeshProperty<std::vector<Real>>(RGMB::duct_halfpitches, mg_name);
  if (duct_halfpitch.size() > 0)
  {
    printMetadataToConsole<std::vector<Real>>(RGMB::duct_halfpitches, mg_name);
    print2dMetadataToConsole<subdomain_id_type>(RGMB::duct_region_ids, mg_name);
  }

  const auto is_single_pin = getMeshProperty<bool>(RGMB::is_single_pin, mg_name);
  const auto is_homogenized = getMeshProperty<bool>(RGMB::is_homogenized, mg_name);

  // Print metadata specific to assemblies defined as a lattice of pins
  if (!is_single_pin)
  {
    printMetadataToConsole<std::vector<std::string>>(RGMB::pin_names, mg_name);
    print2dMetadataToConsole<int>(RGMB::pin_lattice, mg_name);

    // Print information about constituent pins if this is the first function call
    if (first_function_call)
    {
      const auto assembly_pin_names =
          getMeshProperty<std::vector<std::string>>(RGMB::pin_names, mg_name);
      for (const auto & pin_name : assembly_pin_names)
        printReactorMetadata("pin", pin_name, false);
    }
  }
  // Print metadata specific to assemblies defined as a single pin
  else if (!is_homogenized)
  {
    const auto ring_radii = getMeshProperty<std::vector<Real>>(RGMB::ring_radii, mg_name);
    if (ring_radii.size() > 0)
    {
      printMetadataToConsole<std::vector<Real>>(RGMB::ring_radii, mg_name);
      print2dMetadataToConsole<subdomain_id_type>(RGMB::ring_region_ids, mg_name);
    }
  }
}

void
ReactorGeometryMeshBuilderBase::printPinMetadata(const std::string mg_name)
{
  printMetadataToConsole<subdomain_id_type>(RGMB::pin_type, mg_name);
  printMetadataToConsole<Real>(RGMB::pitch, mg_name);

  const auto ring_radii = getMeshProperty<std::vector<Real>>(RGMB::ring_radii, mg_name);
  if (ring_radii.size() > 0)
  {
    printMetadataToConsole<std::vector<Real>>(RGMB::ring_radii, mg_name);
    print2dMetadataToConsole<subdomain_id_type>(RGMB::ring_region_ids, mg_name);
  }

  printMetadataToConsole<std::vector<subdomain_id_type>>(RGMB::background_region_id, mg_name);

  const auto duct_halfpitch = getMeshProperty<std::vector<Real>>(RGMB::duct_halfpitches, mg_name);
  if (duct_halfpitch.size() > 0)
  {
    printMetadataToConsole<std::vector<Real>>(RGMB::duct_halfpitches, mg_name);
    print2dMetadataToConsole<subdomain_id_type>(RGMB::duct_region_ids, mg_name);
  }
}

void
ReactorGeometryMeshBuilderBase::printGlobalReactorMetadata()
{
  printMetadataToConsole<int>(RGMB::mesh_dimensions, _reactor_params);
  printMetadataToConsole<std::string>(RGMB::mesh_geometry, _reactor_params);
  const auto mesh_dimensions = getReactorParam<int>(RGMB::mesh_dimensions);
  if (mesh_dimensions == 3)
  {
    printMetadataToConsole<std::vector<Real>>(RGMB::axial_mesh_sizes, _reactor_params);
    printMetadataToConsole<std::vector<unsigned int>>(RGMB::axial_mesh_intervals, _reactor_params);
  }
}

template <typename T>
void
ReactorGeometryMeshBuilderBase::printMetadataToConsole(const std::string metadata_name,
                                                       const std::string mg_name)
{
  _console << "  " << metadata_name << ": "
           << Moose::stringify(getMeshProperty<T>(metadata_name, mg_name)) << std::endl;
}

template <typename T>
void
ReactorGeometryMeshBuilderBase::print2dMetadataToConsole(const std::string metadata_name,
                                                         const std::string mg_name)
{
  const auto metadata_value = getMeshProperty<std::vector<std::vector<T>>>(metadata_name, mg_name);
  _console << "  " << metadata_name << ":" << std::endl;
  for (const auto & row : metadata_value)
    _console << "    " << Moose::stringify(row) << std::endl;
}
