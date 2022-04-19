//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoreMeshGenerator.h"

#include "MooseApp.h"
#include "Factory.h"
#include "libmesh/elem.h"

registerMooseObject("ReactorApp", CoreMeshGenerator);

InputParameters
CoreMeshGenerator::validParams()
{
  auto params = MeshGenerator::validParams();

  params.addRequiredParam<std::vector<MeshGeneratorName>>(
      "inputs", "The AssemblyMeshGenerators that form the components of the assembly.");

  params.addParam<MeshGeneratorName>(
      "dummy_assembly_name",
      "dummy",
      "The place holder name in \"inputs\" that indicates an empty position.");

  params.addRequiredParam<std::vector<std::vector<unsigned int>>>(
      "pattern",
      "A double-indexed array starting with the upper-left corner where the index"
      "represents the layout of input assemblies in the core lattice.");

  params.addParam<bool>("extrude",
                        false,
                        "Determines if this is the final step in the geometry construction"
                        " and extrudes the 2D geometry to 3D. If this is true then this mesh "
                        "cannot be used in further mesh building in the Reactor workflow");

  params.addClassDescription("This CoreMeshGenerator object is designed to generate a core-like "
                             "structure, with IDs, from a reactor geometry. "
                             "The core-like structure consists of a pattern of assembly-like "
                             "structures generated with AssemblyMeshGenerator "
                             "and is permitted to have \"empty\" locations. The size and spacing "
                             "of the assembly-like structures is defined, and "
                             "enforced by declaration in the ReactorMeshParams.");

  return params;
}

CoreMeshGenerator::CoreMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _inputs(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _empty_key(getParam<MeshGeneratorName>("dummy_assembly_name")),
    _pattern(getParam<std::vector<std::vector<unsigned int>>>("pattern")),
    _extrude(getParam<bool>("extrude"))
{
  /* Below, we get the ReactorMeshParams info from one of the pin inputs.
     The reactor params gives the axial info if we are to extrude as well as the assembly pitch
     that the pin pattern is to be checked against.
     We then create the PatternedMesh subgenerators and pass the parameter info needed for the
     inputs. Finally, if the mesh is to be extruded we create the FancyExtruderGenerator
     subgenerator and the PlaneIDMeshGenerator subgenerator to handle this.
  */
  MeshGeneratorName _reactor_params =
      MeshGeneratorName(getMeshProperty<std::string>("reactor_params_name", _inputs[0]));

  // Ensure that the user has supplied the correct info for conformal mesh generation
  if (!hasMeshProperty("mesh_dimensions", _reactor_params) ||
      !hasMeshProperty("mesh_geometry", _reactor_params))
    mooseError("The reactor_params input must be a ReactorMeshParams type MeshGenerator\n Please "
               "check that a valid definition and name of ReactorMeshParams has been provided to "
               "the input PinMeshGenerators.");
  else
  {
    _geom_type = getMeshProperty<std::string>("mesh_geometry", _reactor_params);
    _mesh_dimensions = getMeshProperty<int>("mesh_dimensions", _reactor_params);
  }
  if (_extrude && getMeshProperty<unsigned int>("mesh_dimensions", _reactor_params) != 3)
    mooseError("This is a 2 dimensional mesh, you cannot extrude it. Check your ReactorMeshParams "
               "inputs\n");
  if (_extrude && (!hasMeshProperty("top_boundary_id", _reactor_params) ||
                   !hasMeshProperty("bottom_boundary_id", _reactor_params)))
    mooseError("Both top_boundary_id and bottom_boundary_id must be provided in ReactorMeshParams "
               "if using extruded geometry");
  if (!hasMeshProperty("radial_boundary_id", _reactor_params))
    mooseError("radial_boundary_id must be provided in ReactorMeshParams for CoreMeshGenerators");

  std::size_t empty_pattern_loc = 0;
  bool make_empty = false;
  for (auto assembly : _inputs)
  {
    if (assembly != _empty_key)
    {
      ++empty_pattern_loc;
      if (getMeshProperty<bool>("extruded", assembly))
        mooseError("Assemblies that have already been extruded cannot be used in CoreMeshGenerator "
                   "definition.\n");
    }
    else
    {
      // Found dummy assembly in input assembly names
      make_empty = true;
      for (const auto i : make_range(_pattern.size()))
      {
        for (const auto j : make_range(_pattern[i].size()))
        {
          // Found dummy assembly in input lattice definition
          if (_pattern[i][j] == empty_pattern_loc)
            _empty_pos = true;
        }
      }
    }
  }

  if (_geom_type == "Square")
  {
    // create a dummy assembly that is 1 assembly sized element that will get deleted later
    if (make_empty)
    {
      Real pitch = getMeshProperty<Real>("assembly_pitch", _reactor_params);

      auto params = _app.getFactory().getValidParams("PolygonConcentricCircleMeshGenerator");
      params.set<unsigned int>("num_sides") = 4;
      params.set<std::vector<unsigned int>>("num_sectors_per_side") =
          std::vector<unsigned int>(4, 2);
      params.set<Real>("polygon_size") = pitch / 2.0;
      params.set<std::vector<subdomain_id_type>>("background_block_ids") =
          std::vector<subdomain_id_type>{UINT16_MAX - 1};

      addMeshSubgenerator(
          "PolygonConcentricCircleMeshGenerator", std::string(_empty_key) + "_circle", params);

      // Rotate assembly to be square rather than diamond
      params = _app.getFactory().getValidParams("TransformGenerator");
      params.set<MeshGeneratorName>("input") = std::string(_empty_key) + "_circle";
      params.set<MooseEnum>("transform") = 4;
      params.set<RealVectorValue>("vector_value") = RealVectorValue(0, 0, 45);

      addMeshSubgenerator("TransformGenerator", std::string(_empty_key), params);
    }
    {
      auto params = _app.getFactory().getValidParams("CartesianIDPatternedMeshGenerator");

      params.set<std::string>("id_name") = "assembly_id";
      params.set<MooseEnum>("assign_type") =
          "cell"; // give elems IDs relative to position in assembly
      params.set<std::vector<MeshGeneratorName>>("inputs") = _inputs;
      params.set<std::vector<std::vector<unsigned int>>>("pattern") = _pattern;
      if (make_empty)
        params.set<std::vector<MeshGeneratorName>>("exclude_id") =
            std::vector<MeshGeneratorName>{_empty_key};

      params.set<BoundaryName>("top_boundary") = "10000";
      params.set<BoundaryName>("left_boundary") = "10000";
      params.set<BoundaryName>("bottom_boundary") = "10000";
      params.set<BoundaryName>("right_boundary") = "10000";

      addMeshSubgenerator("CartesianIDPatternedMeshGenerator", name() + "_lattice", params);
    }
    {
      auto params = _app.getFactory().getValidParams("SideSetsFromNormalsGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_lattice";
      params.set<std::vector<Point>>("normals") = {
          {1, 0, 0},
          {-1, 0, 0},
          {0, 1, 0},
          {0, -1, 0}}; // normal directions over which to define boundaries
      params.set<bool>("fixed_normal") = true;
      params.set<bool>("replace") = false;
      params.set<std::vector<BoundaryName>>("new_boundary") = {
          "tmp_left", "tmp_right", "tmp_top", "tmp_bottom"};

      _build_mesh = &addMeshSubgenerator("SideSetsFromNormalsGenerator", name() + "_bds", params);
    }
    {
      auto params = _app.getFactory().getValidParams("RenameBoundaryGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_bds";
      params.set<std::vector<BoundaryName>>("old_boundary") = {
          "tmp_left", "tmp_right", "tmp_top", "tmp_bottom"};
      params.set<std::vector<BoundaryName>>("new_boundary") =
          std::vector<BoundaryName>(4, "outer_core");

      _build_mesh = &addMeshSubgenerator("RenameBoundaryGenerator", name() + "_pattern", params);
    }
    //***Add assembly duct around PatternedMesh
  }
  else
  {
    // Hex geometry
    // create a dummy assembly that is a renamed version of one of the inputs
    if (make_empty)
    {
      {
        auto params = _app.getFactory().getValidParams(
            "HexagonConcentricCircleAdaptiveBoundaryMeshGenerator");

        params.set<Real>("hexagon_size") =
            getMeshProperty<Real>("assembly_pitch", _reactor_params) / 2.0;
        params.set<std::vector<unsigned int>>("num_sectors_per_side") =
            std::vector<unsigned int>(6, 2);
        params.set<std::vector<unsigned int>>("sides_to_adapt") = std::vector<unsigned int>{0};
        params.set<std::vector<MeshGeneratorName>>("inputs") =
            std::vector<MeshGeneratorName>{_inputs[0]};
        params.set<std::vector<subdomain_id_type>>("background_block_ids") =
            std::vector<subdomain_id_type>{UINT16_MAX - 1};

        addMeshSubgenerator("HexagonConcentricCircleAdaptiveBoundaryMeshGenerator",
                            std::string(_empty_key),
                            params);
      }
    }
    {
      auto params = _app.getFactory().getValidParams("HexIDPatternedMeshGenerator");

      params.set<std::string>("id_name") = "assembly_id";
      params.set<MooseEnum>("assign_type") =
          "cell"; // give elems IDs relative to position in assembly
      params.set<std::vector<MeshGeneratorName>>("inputs") = _inputs;
      params.set<std::vector<std::vector<unsigned int>>>("pattern") = _pattern;
      params.set<MooseEnum>("pattern_boundary") = "none";
      params.set<bool>("generate_core_metadata") = true;
      if (make_empty)
      {
        params.set<std::vector<MeshGeneratorName>>("exclude_id") =
            std::vector<MeshGeneratorName>{_empty_key};
      }

      const auto radial_boundary =
          getMeshProperty<boundary_id_type>("radial_boundary_id", _reactor_params);
      params.set<boundary_id_type>("external_boundary_id") = radial_boundary;
      params.set<std::string>("external_boundary_name") = "outer_core";

      _build_mesh =
          &addMeshSubgenerator("HexIDPatternedMeshGenerator", name() + "_pattern", params);
    }
  }
  if (_empty_pos)
  {
    auto params = _app.getFactory().getValidParams("BlockDeletionGenerator");

    params.set<SubdomainID>("block_id") = UINT16_MAX - 1;
    params.set<MeshGeneratorName>("input") = name() + "_pattern";
    params.set<BoundaryName>("new_boundary") = "outer_core";

    _build_mesh = &addMeshSubgenerator("BlockDeletionGenerator", name() + "_deleted", params);
  }

  // transferring reactor parameters to core mesh
  declareMeshProperty("reactor_params_name", std::string(_reactor_params));

  for (auto assembly : _inputs)
  {
    if (assembly != _empty_key)
    {
      std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>> pin_map =
          getMeshProperty<std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>>(
              "pin_id_map", assembly);
      for (auto pin = pin_map.begin(); pin != pin_map.end(); ++pin)
      {
        if (_id_map.find(pin->first) == _id_map.end())
          _id_map.insert(std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
              pin->first, pin->second));
        else if (pin->second != _id_map.find(pin->first)->second)
          mooseError("Multiple region definitions for the same pin type. Check pin_type ids.\n");
      }

      if (_geom_type == "Hex")
      {
        subdomain_id_type assembly_type =
            getMeshProperty<subdomain_id_type>("assembly_type_id", assembly);
        if (_background_id_map.find(assembly_type) == _background_id_map.end())
        {
          std::vector<subdomain_id_type> background_regions =
              getMeshProperty<std::vector<subdomain_id_type>>("background_region_ids", assembly);
          std::vector<std::vector<subdomain_id_type>> duct_regions =
              getMeshProperty<std::vector<std::vector<subdomain_id_type>>>("duct_region_ids",
                                                                           assembly);
          _background_id_map.insert(std::pair<subdomain_id_type, std::vector<subdomain_id_type>>(
              assembly_type, background_regions));
          _duct_id_map.insert(
              std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
                  assembly_type, duct_regions));
        }
      }
    }
  }
  declareMeshProperty("pin_id_map", _id_map);

  declareMeshProperty("assembly_pitch", getMeshProperty<Real>("assembly_pitch", _reactor_params));

  if (_extrude && _mesh_dimensions == 3)
  {
    std::vector<Real> axial_boundaries =
        getMeshProperty<std::vector<Real>>("axial_boundaries", _reactor_params);
    const auto top_boundary = getMeshProperty<boundary_id_type>("top_boundary_id", _reactor_params);
    const auto bottom_boundary =
        getMeshProperty<boundary_id_type>("bottom_boundary_id", _reactor_params);
    {
      declareMeshProperty("extruded", true);
      auto params = _app.getFactory().getValidParams("FancyExtruderGenerator");

      params.set<MeshGeneratorName>("input") =
          _empty_pos ? name() + "_deleted" : name() + "_pattern";
      params.set<Point>("direction") = Point(0, 0, 1);
      params.set<std::vector<unsigned int>>("num_layers") =
          getMeshProperty<std::vector<unsigned int>>("axial_mesh_intervals", _reactor_params);
      params.set<std::vector<Real>>("heights") = axial_boundaries;
      params.set<boundary_id_type>("bottom_boundary") = bottom_boundary;
      params.set<boundary_id_type>("top_boundary") = top_boundary;

      addMeshSubgenerator("FancyExtruderGenerator", name() + "_extruded", params);
    }

    {
      auto params = _app.getFactory().getValidParams("RenameBoundaryGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_extruded";
      params.set<std::vector<BoundaryName>>("old_boundary") = {
          std::to_string(top_boundary),
          std::to_string(bottom_boundary)}; // hard coded boundary IDs in patterned mesh generator
      params.set<std::vector<BoundaryName>>("new_boundary") = {"top", "bottom"};

      addMeshSubgenerator("RenameBoundaryGenerator", name() + "_change_plane_name", params);
    }

    {
      auto params = _app.getFactory().getValidParams("PlaneIDMeshGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_change_plane_name";

      std::vector<Real> plane_heights{0};
      for (Real z : axial_boundaries)
      {
        plane_heights.push_back(z + plane_heights.back());
      }
      params.set<std::vector<Real>>("plane_coordinates") = plane_heights;

      std::string plane_id_name = "plane_id";
      params.set<std::string>("id_name") = "plane_id";

      _build_mesh = &addMeshSubgenerator("PlaneIDMeshGenerator", name() + "_extrudedIDs", params);
    }
  }
  else
    declareMeshProperty("extruded", false);
}

std::unique_ptr<MeshBase>
CoreMeshGenerator::generate()
{
  // This generate() method will be called once the subgenerators that we depend on are
  // called. This is where we set the region ids and element integers.
  if (_extrude)
  {
    // swap the region ids on the subdomain ids to the correct ones
    // for their axial layer
    std::string plane_id_name = "plane_id";
    std::string pin_type_id_name = "pin_type_id";
    std::string assembly_type_id_name = "assembly_type_id";

    unsigned int ptid_int = (*_build_mesh)->get_elem_integer_index(pin_type_id_name);
    unsigned int atid_int = (*_build_mesh)->get_elem_integer_index(assembly_type_id_name);
    unsigned int pid_int = (*_build_mesh)->get_elem_integer_index(plane_id_name);

    for (auto & elem : (*_build_mesh)->active_element_ptr_range())
    {
      dof_id_type z_id = elem->get_extra_integer(pid_int);
      dof_id_type pt_id = elem->get_extra_integer(ptid_int);
      subdomain_id_type r_id = elem->subdomain_id();

      // Going through the elements in the mesh checking the pin_type_ids to axial
      // region ids
      if (_id_map.find(pt_id) == _id_map.end())
      {
        // element is in an assembly duct or background region since it doesn't have a pin type id
        // that matches one in the map. What region it is in the periphery is then stored in the
        // pt_id
        dof_id_type at_id = elem->get_extra_integer(atid_int);
        unsigned int peripheral_index = (UINT16_MAX - 1) - pt_id;
        if (peripheral_index == 0)
          // background region
          elem->subdomain_id() = _background_id_map.find(at_id)->second[z_id];
        else
          // duct region
          elem->subdomain_id() = _duct_id_map.find(at_id)->second[z_id][peripheral_index - 1];
      }
      else
      {
        // region is in a pin so grab the different axial region ids and swap them
        // since during extrusion all regions are given the same ID as the 2D layer
        for (const auto i : make_range((_id_map.at(pt_id))[0].size()))
        {
          // swap subdomain region ids if they are different
          if (r_id == _id_map.at(pt_id)[0][i] &&
              _id_map.at(pt_id)[0][i] != _id_map.at(pt_id)[z_id][i])
            elem->subdomain_id() = _id_map.at(pt_id)[z_id][i];
        }
      }
    }
  }

  std::string region_id_name = "region_id";
  if (!(*_build_mesh)->has_elem_integer(region_id_name))
  {
    mooseError("Expected mesh inputs to have the extra integer id: region_id.\n");
  }

  unsigned int rid = (*_build_mesh)->get_elem_integer_index(region_id_name);

  std::map<subdomain_id_type, std::string> subdomain_name_map;
  for (auto & elem : (*_build_mesh)->active_element_ptr_range())
  {
    elem->set_extra_integer(rid, elem->subdomain_id());

    if (subdomain_name_map.find(elem->subdomain_id()) == subdomain_name_map.end())
      subdomain_name_map.insert(std::pair<subdomain_id_type, std::string>(
          elem->subdomain_id(), std::to_string(elem->subdomain_id())));
  }

  (*_build_mesh)->set_subdomain_name_map() = subdomain_name_map;

  // Sideset 10000 does not get stitched properly when BlockDeletionGenerator
  // is used for deleting dummy assemblies. This block copies missing sides
  // into sideset 10000 from sideset "outer_core"
  BoundaryInfo & boundary_info = (*_build_mesh)->get_boundary_info();
  boundary_id_type source_id = MooseMeshUtils::getBoundaryIDs(**_build_mesh, {"outer_core"}, true)[0];
  boundary_id_type target_id = 10000;
  const auto sideset_map = boundary_info.get_sideset_map();

  for (const auto & [elem, id_pair] : sideset_map) {
    const auto side_id = id_pair.first;
    const auto sideset_id = id_pair.second;

    // Filter all sides that belong to "outer_core" sideset
    if (sideset_id == source_id)
    {
      auto mm_it = sideset_map.equal_range(elem);
      bool found = false;
      // Check if side is defined in sideset 10000
      for (auto it = mm_it.first; it != mm_it.second; it++)
      {
        if (it->second.first == side_id && it->second.second == target_id)
          found = true;
      }
      // Add side if not found in sideset 10000
      if (!found)
        boundary_info.add_side(elem, side_id, target_id);
    }
  }

  return std::move(*_build_mesh);
}
