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

  params.addParam<bool>("show_rgmb_metadata", false,
                        "Print out RGMB-related metadata to console output");
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

  // Check value of show_rgmb_metadata is valid
  if (getParam<bool>("show_rgmb_metadata") && !getReactorParam<bool>("generate_rgmb_metadata"))
    paramError("show_rgmb_metadata", "`ReactorMeshParams/generate_rgmb_metadata` must be set to true in order for show_rgmb_metadata to be true");
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
ReactorGeometryMeshBuilderBase::generateGlobalReactorMetadata(const std::string prefix)
{
  declareMeshProperty(prefix + "_mesh_dimensions", getReactorParam<int>("mesh_dimensions"));
  declareMeshProperty(prefix + "_mesh_geometry", getReactorParam<std::string>("mesh_geometry"));
  declareMeshProperty(prefix + "_axial_boundaries", getReactorParam<std::vector<Real>>("axial_boundaries"));
  declareMeshProperty(prefix + "_axial_mesh_intervals", getReactorParam<std::vector<unsigned int>>("axial_mesh_intervals"));
}

void
ReactorGeometryMeshBuilderBase::copyPinMetadata(const std::string input_name, const unsigned int pin_type_id)
{
  std::string metadata_prefix = "pin_" + std::to_string(pin_type_id);

  declareMeshProperty(metadata_prefix + "_pitch", getMeshProperty<Real>(metadata_prefix + "_pitch", input_name));
  declareMeshProperty(metadata_prefix + "_ring_radii", getMeshProperty<std::vector<Real>>(metadata_prefix + "_ring_radii", input_name));
  declareMeshProperty(metadata_prefix + "_ring_region_ids", getMeshProperty<std::vector<std::vector<subdomain_id_type>>>(metadata_prefix + "_ring_region_ids", input_name));
  declareMeshProperty(metadata_prefix + "_background_region_id", getMeshProperty<std::vector<subdomain_id_type>>(metadata_prefix + "_background_region_id", input_name));
  declareMeshProperty(metadata_prefix + "_duct_halfpitches", getMeshProperty<std::vector<Real>>(metadata_prefix + "_duct_halfpitches", input_name));
  declareMeshProperty(metadata_prefix + "_duct_region_ids", getMeshProperty<std::vector<std::vector<subdomain_id_type>>>(metadata_prefix + "_duct_region_ids", input_name));
}

void
ReactorGeometryMeshBuilderBase::copyAssemblyMetadata(const std::string input_name, const unsigned int assembly_type_id)
{
  std::string metadata_prefix = "assembly_" + std::to_string(assembly_type_id);

  declareMeshProperty(metadata_prefix + "_pitch", getMeshProperty<Real>(metadata_prefix + "_pitch", input_name));
  declareMeshProperty(metadata_prefix + "_background_region_id", getMeshProperty<std::vector<subdomain_id_type>>(metadata_prefix + "_background_region_id", input_name));
  declareMeshProperty(metadata_prefix + "_duct_halfpitches", getMeshProperty<std::vector<Real>>(metadata_prefix + "_duct_halfpitches", input_name));
  declareMeshProperty(metadata_prefix + "_duct_region_ids", getMeshProperty<std::vector<std::vector<subdomain_id_type>>>(metadata_prefix + "_duct_region_ids", input_name));
  const auto is_homogenized = getMeshProperty<bool>(metadata_prefix + "_is_homogenized", input_name);
  const auto is_single_pin = getMeshProperty<bool>(metadata_prefix + "_is_single_pin", input_name);
  declareMeshProperty(metadata_prefix + "_is_homogenized", is_homogenized);
  declareMeshProperty(metadata_prefix + "_is_single_pin", is_single_pin);

  // Define metadata specific to assemblies defined as a lattice of pins
  if (!is_single_pin)
  {
    declareMeshProperty(metadata_prefix + "_pin_types", getMeshProperty<std::set<unsigned int>>(metadata_prefix + "_pin_types", input_name));
    declareMeshProperty(metadata_prefix + "_lattice", getMeshProperty<std::vector<std::vector<int>>>(metadata_prefix + "_lattice", input_name));
  }
  // Defined metadata specific to assemblies defined as a single heterogeneous pin
  else if (!is_homogenized)
  {
    declareMeshProperty(metadata_prefix + "_ring_radii", getMeshProperty<std::vector<Real>>(metadata_prefix + "_ring_radii", input_name));
  declareMeshProperty(metadata_prefix + "_ring_region_ids", getMeshProperty<std::vector<std::vector<subdomain_id_type>>>(metadata_prefix + "_ring_region_ids", input_name));
  }
}

void
ReactorGeometryMeshBuilderBase::printReactorMetadata(const std::string metadata_prefix, bool first_function_call)
{
  if (metadata_prefix == "core")
  {
    if (first_function_call)
    {
      Moose::out << "Core-level metadata defined using Reactor Geometry Mesh Builder:" << std::endl;
      printGlobalReactorMetadata(metadata_prefix);
    }
    printCoreMetadata(metadata_prefix, first_function_call);
  }
  else
  {
    std::string prefix_substring = metadata_prefix.substr(0, metadata_prefix.find("_"));
    if (prefix_substring == "assembly")
    {
      if (first_function_call)
      {
        Moose::out << "Assembly-level metadata defined using Reactor Geometry Mesh Builder:" << std::endl;
        printGlobalReactorMetadata(metadata_prefix);
      }
      printAssemblyMetadata(metadata_prefix, first_function_call);
    }
    else if (prefix_substring == "pin")
    {
      if (first_function_call)
      {
        Moose::out << "Pin-level metadata defined using Reactor Geometry Mesh Builder:" << std::endl;
        printGlobalReactorMetadata(metadata_prefix);
      }
      printPinMetadata(metadata_prefix);
    }
  }
  if (first_function_call)
    Moose::out << std::endl;
}

void
ReactorGeometryMeshBuilderBase::printCoreMetadata(const std::string prefix, bool first_function_call)
{
  bool has_mesh_periphery = hasMeshProperty<Real>(prefix + "_peripheral_ring_radius");
  if (has_mesh_periphery)
  {
    printMetadataToConsole<Real>(prefix + "_peripheral_ring_radius");
    printMetadataToConsole<subdomain_id_type>(prefix + "_peripheral_ring_region_id");
  }

  printMetadataToConsole<std::set<unsigned int>>(prefix + "_assembly_types");
  print2dMetadataToConsole<int>(prefix + "_lattice");

  if (first_function_call)
  {
    const auto core_assembly_types = getMeshProperty<std::set<unsigned int>>(prefix + "_assembly_types");
    for (const auto & assembly_type_id : core_assembly_types)
      printReactorMetadata("assembly_" + Moose::stringify(assembly_type_id), false);

    const auto core_pin_types = getMeshProperty<std::set<unsigned int>>(prefix + "_pin_types");
    for (const auto & assembly_type_id : core_pin_types)
      printReactorMetadata("pin_" + Moose::stringify(assembly_type_id), false);
  }
}

void
ReactorGeometryMeshBuilderBase::printAssemblyMetadata(const std::string prefix, bool first_function_call)
{
  printMetadataToConsole<Real>(prefix + "_pitch");
  printMetadataToConsole<bool>(prefix + "_is_homogenized");
  printMetadataToConsole<bool>(prefix + "_is_single_pin");
  printMetadataToConsole<std::vector<subdomain_id_type>>(prefix + "_background_region_id");

  const auto duct_halfpitch = getMeshProperty<std::vector<Real>>(prefix + "_duct_halfpitches");
  if (duct_halfpitch.size() > 0)
  {
    printMetadataToConsole<std::vector<Real>>(prefix + "_duct_halfpitches");
    print2dMetadataToConsole<subdomain_id_type>(prefix + "_duct_region_ids");
  }

  const auto is_single_pin = getMeshProperty<bool>(prefix + "_is_single_pin");
  const auto is_homogenized = getMeshProperty<bool>(prefix + "_is_homogenized");

  // Print metadata specific to assemblies defined as a lattice of pins
  if (!is_single_pin)
  {
    printMetadataToConsole<std::set<unsigned int>>(prefix + "_pin_types");
    print2dMetadataToConsole<int>(prefix + "_lattice");

    // Print information about constituent pins if this is the first function call
    if (first_function_call)
    {
      const auto assembly_pin_types = getMeshProperty<std::set<unsigned int>>(prefix + "_pin_types");
      for (const auto & pin_type_id : assembly_pin_types)
        printReactorMetadata("pin_" + Moose::stringify(pin_type_id), false);
    }
  }
  // Print metadata specific to assemblies defined as a single pin
  else if (!is_homogenized)
  {
    const auto ring_radii = getMeshProperty<std::vector<Real>>(prefix + "_ring_radii");
    if (ring_radii.size() > 0)
    {
      printMetadataToConsole<std::vector<Real>>(prefix + "_ring_radii");
      print2dMetadataToConsole<subdomain_id_type>(prefix + "_ring_region_ids");
    }
  }
}

void
ReactorGeometryMeshBuilderBase::printPinMetadata(const std::string prefix)
{
  printMetadataToConsole<Real>(prefix + "_pitch");

  const auto ring_radii = getMeshProperty<std::vector<Real>>(prefix + "_ring_radii");
  if (ring_radii.size() > 0)
  {
    printMetadataToConsole<std::vector<Real>>(prefix + "_ring_radii");
    print2dMetadataToConsole<subdomain_id_type>(prefix + "_ring_region_ids");
  }

  printMetadataToConsole<std::vector<subdomain_id_type>>(prefix + "_background_region_id");

  const auto duct_halfpitch = getMeshProperty<std::vector<Real>>(prefix + "_duct_halfpitches");
  if (duct_halfpitch.size() > 0)
  {
    printMetadataToConsole<std::vector<Real>>(prefix + "_duct_halfpitches");
    print2dMetadataToConsole<subdomain_id_type>(prefix + "_duct_region_ids");
  }
}

void
ReactorGeometryMeshBuilderBase::printGlobalReactorMetadata(const std::string prefix)
{
  printMetadataToConsole<int>(prefix + "_mesh_dimensions");
  printMetadataToConsole<std::string>(prefix + "_mesh_geometry");
  printMetadataToConsole<std::vector<Real>>(prefix + "_axial_boundaries");
  printMetadataToConsole<std::vector<unsigned int>>(prefix + "_axial_mesh_intervals");
}

template <typename T> void
ReactorGeometryMeshBuilderBase::printMetadataToConsole(const std::string metadata_name)
{
  Moose::out << "  " << metadata_name << ": " << Moose::stringify(getMeshProperty<T>(metadata_name)) << std::endl;
}

template <typename T> void
ReactorGeometryMeshBuilderBase::print2dMetadataToConsole(const std::string metadata_name)
{
  const auto metadata_value = getMeshProperty<std::vector<std::vector<T>>>(metadata_name);
  Moose::out << "  " << metadata_name << ":" << std::endl;
  for (const auto & row : metadata_value)
    Moose::out << "    " << Moose::stringify(row) << std::endl;
}
