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
#include "MooseMeshUtils.h"
#include "Factory.h"
#include "libmesh/elem.h"

registerMooseObject("ReactorApp", CoreMeshGenerator);

InputParameters
CoreMeshGenerator::validParams()
{
  auto params = ReactorGeometryMeshBuilderBase::validParams();

  params.addRequiredParam<std::vector<MeshGeneratorName>>(
      "inputs",
      "The AssemblyMeshGenerator and ControlDrumMeshGenerator objects that form the components of "
      "the assembly.");

  params.addParam<std::string>(
      "dummy_assembly_name",
      "dummy",
      "The place holder name in \"inputs\" that indicates an empty position.");

  params.addRequiredParam<std::vector<std::vector<unsigned int>>>(
      "pattern",
      "A double-indexed array starting with the upper-left corner where the index"
      "represents the layout of input assemblies in the core lattice.");
  params.addParam<bool>(
      "mesh_periphery", false, "Determines if the core periphery should be meshed.");
  MooseEnum periphery_mesher("triangle quad_ring", "triangle");
  params.addParam<MooseEnum>("periphery_generator",
                             periphery_mesher,
                             "The meshgenerator to use when meshing the core boundary.");

  // Periphery meshing interface
  params.addRangeCheckedParam<Real>(
      "outer_circle_radius", 0, "outer_circle_radius>=0", "Radius of outer circle boundary.");
  params.addRangeCheckedParam<unsigned int>(
      "outer_circle_num_segments",
      0,
      "outer_circle_num_segments>=0",
      "Number of radial segments to subdivide outer circle boundary.");
  params.addRangeCheckedParam<unsigned int>(
      "periphery_num_layers",
      1,
      "periphery_num_layers>0",
      "Number of layers to subdivide the periphery boundary.");
  params.addParam<std::string>(
      "periphery_block_name", RGMB::CORE_BLOCK_NAME_PREFIX, "Block name for periphery zone.");
  params.addParam<subdomain_id_type>(
      "periphery_region_id",
      -1,
      "ID for periphery zone for assignment of region_id extra element id.");
  params.addRangeCheckedParam<Real>(
      "desired_area",
      0,
      "desired_area>=0",
      "Desired (maximum) triangle area, or 0 to skip uniform refinement");
  params.addParam<std::string>(
      "desired_area_func",
      std::string(),
      "Desired (local) triangle area as a function of x,y; omit to skip non-uniform refinement");
  params.addParamNamesToGroup("periphery_block_name periphery_region_id outer_circle_radius "
                              "mesh_periphery periphery_generator",
                              "Periphery Meshing");
  params.addParamNamesToGroup("outer_circle_num_segments desired_area desired_area_func",
                              "Periphery Meshing: PTMG specific");
  params.addParamNamesToGroup("periphery_num_layers", "Periphery Meshing: PRMG specific");
  // end meshing interface

  params.addParam<bool>("extrude",
                        false,
                        "Determines if this is the final step in the geometry construction"
                        " and extrudes the 2D geometry to 3D. If this is true then this mesh "
                        "cannot be used in further mesh building in the Reactor workflow");

  params.addClassDescription(
      "This CoreMeshGenerator object is designed to generate a core-like "
      "structure, with IDs, from a reactor geometry. "
      "The core-like structure consists of a pattern of assembly-like "
      "structures generated with AssemblyMeshGenerator and/or ControlDrumMeshGenerator "
      "and is permitted to have \"empty\" locations. The size and spacing "
      "of the assembly-like structures is defined, and "
      "enforced by declaration in the ReactorMeshParams.");
  // depletion id generation params are added
  addDepletionIDParams(params);

  return params;
}

CoreMeshGenerator::CoreMeshGenerator(const InputParameters & parameters)
  : ReactorGeometryMeshBuilderBase(parameters),
    _inputs(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _empty_key(getParam<std::string>("dummy_assembly_name")),
    _pattern(getParam<std::vector<std::vector<unsigned int>>>("pattern")),
    _extrude(getParam<bool>("extrude")),
    _mesh_periphery(getParam<bool>("mesh_periphery")),
    _periphery_meshgenerator(getParam<MooseEnum>("periphery_generator")),
    _periphery_region_id(getParam<subdomain_id_type>("periphery_region_id")),
    _outer_circle_radius(getParam<Real>("outer_circle_radius")),
    _outer_circle_num_segments(getParam<unsigned int>("outer_circle_num_segments")),
    _periphery_block_name(getParam<std::string>("periphery_block_name")),
    _periphery_num_layers(getParam<unsigned int>("periphery_num_layers")),
    _desired_area(getParam<Real>("desired_area")),
    _desired_area_func(getParam<std::string>("desired_area_func"))
{
  // This sets it so that any mesh that is input with the name _empty_key is considered a "null"
  // mesh, that is, whenever we try to get it with the standard getMesh() API we get a nullptr
  // mesh instead. In the specific case of the CoreMeshGenerator, we use said "null" mesh to
  // represent an empty position
  declareNullMeshName(_empty_key);

  // periphery meshing input checking
  if (_mesh_periphery)
  {
    // missing required input
    if (!parameters.isParamSetByUser("outer_circle_radius"))
    {
      paramError("outer_circle_radius",
                 "Outer circle radius must be specified when using periphery meshing.");
    }
    if (!parameters.isParamSetByUser("periphery_region_id"))
    {
      paramError("periphery_region_id",
                 "Periphery region id must be specified when using periphery meshing.");
    }
    // using PTMG-specific options with PRMG
    if (_periphery_meshgenerator == "quad_ring")
    {
      if (parameters.isParamSetByUser("outer_circle_num_segments"))
      {
        paramError("outer_circle_num_segments",
                   "outer_circle_num_segments cannot be used with PRMG periphery mesher.");
      }
      if (parameters.isParamSetByUser("extra_circle_radii"))
      {
        paramError("extra_circle_radii",
                   "extra_circle_radii cannot be used with PRMG periphery mesher.");
      }
      if (parameters.isParamSetByUser("extra_circle_num_segments"))
      {
        paramError("extra_circle_num_segments",
                   "extra_circle_num_segments cannot be used with PRMG periphery mesher.");
      }
    }
    // using PRMG-specific options with PTMG
    else if (_periphery_meshgenerator == "triangle")
    {
      if (parameters.isParamSetByUser("periphery_num_layers"))
      {
        paramError("periphery_num_layers",
                   "periphery_num_layers cannot be used with PTMG periphery mesher.");
      }
    }
    else
      paramError("periphery_generator",
                 "Provided periphery meshgenerator has not been implemented.");
  }

  MeshGeneratorName first_nondummy_assembly = "";
  MeshGeneratorName reactor_params = "";
  bool assembly_homogenization = false;
  bool pin_as_assembly = false;
  std::map<subdomain_id_type, std::string> global_pin_map_type_to_name;
  std::map<subdomain_id_type, std::string> assembly_map_type_to_name;
  // Check that MG name for reactor params and assembly homogenization schemes are
  // consistent across all assemblies, and there is no overlap in pin_type / assembly_type ids
  for (const auto i : index_range(_inputs))
  {
    // Skip if assembly name is equal to dummy assembly name
    if (_inputs[i] == _empty_key)
      continue;

    // Save properties of first non-dummy assembly to compare to other assemblies
    if (first_nondummy_assembly == "")
    {
      first_nondummy_assembly = MeshGeneratorName(_inputs[i]);
      reactor_params =
          MeshGeneratorName(getMeshProperty<std::string>(RGMB::reactor_params_name, _inputs[i]));
      assembly_homogenization = getMeshProperty<bool>(RGMB::is_homogenized, _inputs[i]);
      pin_as_assembly = getMeshProperty<bool>(RGMB::is_single_pin, _inputs[i]);
    }
    if (getMeshProperty<std::string>(RGMB::reactor_params_name, _inputs[i]) != reactor_params)
      mooseError("The name of all reactor_params objects should be identical across all pins in "
                 "the input assemblies.\n");
    if ((getMeshProperty<bool>(RGMB::is_homogenized, _inputs[i]) != assembly_homogenization) &&
        !getMeshProperty<bool>(RGMB::flexible_assembly_stitching, reactor_params))
      mooseError("In order to stitch heterogeneous assemblies with homogeneous assemblies in "
                 "CoreMeshGenerator, ReactorMeshParams/flexible_assembly_stitching should be set "
                 "to true\n");

    // Check assembly_types across constituent assemblies are uniquely defined
    const auto assembly_type = getMeshProperty<subdomain_id_type>(RGMB::assembly_type, _inputs[i]);
    if (assembly_map_type_to_name.find(assembly_type) != assembly_map_type_to_name.end() &&
        assembly_map_type_to_name[assembly_type] != _inputs[i])
      mooseError(
          "Constituent assemblies have shared assembly_type ids but different names. Each uniquely "
          "defined assembly in AssemblyMeshGenerator must have its own assembly_type id.");
    assembly_map_type_to_name[assembly_type] = _inputs[i];

    // If assembly is composed of pins, check pin_types across all constituent assemblies are
    // uniquely defined
    if (hasMeshProperty<std::vector<std::string>>(RGMB::pin_names, _inputs[i]))
    {
      const auto pin_names = getMeshProperty<std::vector<std::string>>(RGMB::pin_names, _inputs[i]);
      for (const auto & input_pin_name : pin_names)
      {
        const auto pin_type = getMeshProperty<subdomain_id_type>(RGMB::pin_type, input_pin_name);
        if (global_pin_map_type_to_name.find(pin_type) != global_pin_map_type_to_name.end() &&
            global_pin_map_type_to_name[pin_type] != input_pin_name)
          mooseError(
              "Constituent pins within assemblies have shared pin_type ids but different names. "
              "Each uniquely defined pin in AssemblyMeshGenerator must have its own pin_type id.");
        global_pin_map_type_to_name[pin_type] = input_pin_name;
      }
    }
  }

  // Check that there is at least one non-dummy assemby defined in lattice
  if (first_nondummy_assembly == "")
    paramError("inputs", "At least one non-dummy assembly must be defined in input assembly names");

  // Initialize ReactorMeshParams object stored in pin input
  initializeReactorMeshParams(reactor_params);

  _geom_type = getReactorParam<std::string>(RGMB::mesh_geometry);
  _mesh_dimensions = getReactorParam<unsigned int>(RGMB::mesh_dimensions);

  if (_extrude && _mesh_dimensions != 3)
    paramError("extrude",
               "In order to extrude this mesh, ReactorMeshParams/dim needs to be set to 3\n");
  if (_extrude && (!hasReactorParam<boundary_id_type>(RGMB::top_boundary_id) ||
                   !hasReactorParam<boundary_id_type>(RGMB::bottom_boundary_id)))
    mooseError("Both top_boundary_id and bottom_boundary_id must be provided in ReactorMeshParams "
               "if using extruded geometry");
  if (!hasReactorParam<boundary_id_type>(RGMB::radial_boundary_id))
    mooseError("radial_boundary_id must be provided in ReactorMeshParams for CoreMeshGenerators");

  if (parameters.isParamSetByUser("periphery_block_name") &&
      getReactorParam<bool>(RGMB::region_id_as_block_name))
    paramError("periphery_block_name",
               "If ReactorMeshParams/region_id_as_block_name is set, periphery_block_name should "
               "not be specified in CoreMeshGenerator");

  std::size_t empty_pattern_loc = 0;
  bool make_empty = false;
  for (auto assembly : _inputs)
  {
    if (assembly != _empty_key)
    {
      ++empty_pattern_loc;
      if (getMeshProperty<bool>(RGMB::extruded, assembly))
        mooseError("Assemblies that have already been extruded cannot be used in CoreMeshGenerator "
                   "definition.\n");
    }
    else
    {
      // Found dummy assembly in input assembly names
      make_empty = true;
      for (const auto i : index_range(_pattern))
      {
        for (const auto j : index_range(_pattern[i]))
        {
          // Found dummy assembly in input lattice definition
          if (_pattern[i][j] == empty_pattern_loc)
            _empty_pos = true;
        }
      }
    }
  }

  // No subgenerators will be called if option to bypass mesh generators is enabled
  if (!getReactorParam<bool>(RGMB::bypass_meshgen))
  {
    // Check whether flexible stitching should be used for constituent assemblies and throw a
    // warning if flexible stitching option is not enabled
    if (!getReactorParam<bool>(RGMB::flexible_assembly_stitching) &&
        constituentAssembliesNeedFlexibleStiching())
      mooseWarning("Constituent assemblies do not share the same number of nodes at the outer "
                   "boundary. In order to ensure that output mesh does not having hanging nodes, a "
                   "flexible stitching approach should be used by setting "
                   "ReactorMeshParams/flexible_assembly_stitching = true.");

    // Declare that all of the meshes in the "inputs" parameter are to be used by
    // a sub mesh generator.
    declareMeshesForSub("inputs");

    // Stitch assemblies into a hexagonal / Cartesian core lattice
    {
      // create a dummy assembly that is a renamed version of one of the inputs
      if (make_empty)
      {
        {
          if (assembly_homogenization)
          {
            auto params = _app.getFactory().getValidParams("SimpleHexagonGenerator");

            params.set<Real>("hexagon_size") = getReactorParam<Real>(RGMB::assembly_pitch) / 2.0;
            params.set<std::vector<subdomain_id_type>>("block_id") = {
                RGMB::DUMMY_ASSEMBLY_BLOCK_ID};

            addMeshSubgenerator("SimpleHexagonGenerator", std::string(_empty_key), params);
          }
          else
          {
            const auto adaptive_mg_name =
                _geom_type == "Hex" ? "HexagonConcentricCircleAdaptiveBoundaryMeshGenerator"
                                    : "CartesianConcentricCircleAdaptiveBoundaryMeshGenerator";
            auto params = _app.getFactory().getValidParams(adaptive_mg_name);

            const auto assembly_pitch = getReactorParam<Real>(RGMB::assembly_pitch);
            if (_geom_type == "Hex")
            {
              params.set<Real>("hexagon_size") = assembly_pitch / 2.0;
              params.set<std::vector<unsigned int>>("num_sectors_per_side") =
                  std::vector<unsigned int>(6, 2);
            }
            else
            {
              params.set<Real>("square_size") = assembly_pitch;
              params.set<std::vector<unsigned int>>("num_sectors_per_side") =
                  std::vector<unsigned int>(4, 2);
            }
            params.set<std::vector<unsigned int>>("sides_to_adapt") = std::vector<unsigned int>{0};
            params.set<std::vector<MeshGeneratorName>>("meshes_to_adapt_to") =
                std::vector<MeshGeneratorName>{first_nondummy_assembly};
            params.set<std::vector<subdomain_id_type>>("background_block_ids") =
                std::vector<subdomain_id_type>{RGMB::DUMMY_ASSEMBLY_BLOCK_ID};

            addMeshSubgenerator(adaptive_mg_name, std::string(_empty_key), params);
          }
        }
      }
      {
        const auto patterned_mg_name =
            _geom_type == "Hex" ? "PatternedHexMeshGenerator" : "PatternedCartesianMeshGenerator";
        auto params = _app.getFactory().getValidParams(patterned_mg_name);

        params.set<std::vector<std::string>>("id_name") = {"assembly_id"};
        params.set<std::vector<MooseEnum>>("assign_type") = {
            MooseEnum("cell", "cell")}; // give elems IDs relative to position in assembly
        params.set<std::vector<MeshGeneratorName>>("inputs") = _inputs;
        params.set<std::vector<std::vector<unsigned int>>>("pattern") = _pattern;
        params.set<MooseEnum>("pattern_boundary") = "none";
        params.set<bool>("generate_core_metadata") = !pin_as_assembly;
        params.set<bool>("create_outward_interface_boundaries") = false;
        if (make_empty)
        {
          params.set<std::vector<MeshGeneratorName>>("exclude_id") =
              std::vector<MeshGeneratorName>{_empty_key};
        }

        const auto radial_boundary = getReactorParam<boundary_id_type>(RGMB::radial_boundary_id);
        params.set<boundary_id_type>("external_boundary_id") = radial_boundary;
        params.set<BoundaryName>("external_boundary_name") = RGMB::CORE_BOUNDARY_NAME;
        params.set<double>("rotate_angle") = 0.0;

        addMeshSubgenerator(patterned_mg_name, name() + "_pattern", params);
      }
    }
    if (_empty_pos)
    {
      auto params = _app.getFactory().getValidParams("BlockDeletionGenerator");

      params.set<std::vector<SubdomainName>>("block") = {
          std::to_string(RGMB::DUMMY_ASSEMBLY_BLOCK_ID)};
      params.set<MeshGeneratorName>("input") = name() + "_pattern";
      params.set<BoundaryName>("new_boundary") = RGMB::CORE_BOUNDARY_NAME;

      addMeshSubgenerator("BlockDeletionGenerator", name() + "_deleted", params);
    }

    std::string build_mesh_name;

    // Remove outer assembly sidesets created during assembly generation
    {
      // Get outer boundaries of all constituent assemblies based on assembly_type,
      // skipping all dummy assemblies
      std::vector<BoundaryName> boundaries_to_delete = {};
      for (const auto & pattern_x : _pattern)
      {
        for (const auto & pattern_idx : pattern_x)
        {
          const auto assembly_name = _inputs[pattern_idx];
          if (assembly_name == _empty_key)
            continue;
          const auto assembly_id =
              getMeshProperty<subdomain_id_type>(RGMB::assembly_type, assembly_name);
          const BoundaryName boundary_name =
              RGMB::ASSEMBLY_BOUNDARY_NAME_PREFIX + std::to_string(assembly_id);
          if (!std::count(boundaries_to_delete.begin(), boundaries_to_delete.end(), boundary_name))
            boundaries_to_delete.push_back(boundary_name);
        }
      }
      auto params = _app.getFactory().getValidParams("BoundaryDeletionGenerator");

      params.set<MeshGeneratorName>("input") =
          _empty_pos ? name() + "_deleted" : name() + "_pattern";
      params.set<std::vector<BoundaryName>>("boundary_names") = boundaries_to_delete;

      build_mesh_name = name() + "_delbds";
      addMeshSubgenerator("BoundaryDeletionGenerator", build_mesh_name, params);
    }

    for (auto assembly : _inputs)
    {
      if (assembly != _empty_key)
      {
        subdomain_id_type assembly_type =
            getMeshProperty<subdomain_id_type>(RGMB::assembly_type, assembly);
        if (!getMeshProperty<bool>(RGMB::is_control_drum, assembly))
        {
          // For assembly structures, store region ID and block names of assembly regions and
          // constituent pins
          const auto & pin_region_id_map = getMeshProperty<
              std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>>(
              RGMB::pin_region_id_map, assembly);
          for (auto pin = pin_region_id_map.begin(); pin != pin_region_id_map.end(); ++pin)
            _pin_region_id_map.insert(
                std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
                    pin->first, pin->second));

          const auto & pin_block_name_map =
              getMeshProperty<std::map<subdomain_id_type, std::vector<std::vector<std::string>>>>(
                  RGMB::pin_block_name_map, assembly);
          for (auto pin = pin_block_name_map.begin(); pin != pin_block_name_map.end(); ++pin)
            _pin_block_name_map.insert(
                std::pair<subdomain_id_type, std::vector<std::vector<std::string>>>(pin->first,
                                                                                    pin->second));

          // Define background and duct region ID map from constituent assemblies
          if (_background_region_id_map.find(assembly_type) == _background_region_id_map.end())
          {
            // Store region ids and block names associated with duct and background regions for each
            // assembly, in case block names need to be recovered from region ids after
            // multiple assemblies have been stitched together into a core
            std::vector<subdomain_id_type> background_region_ids =
                getMeshProperty<std::vector<subdomain_id_type>>(RGMB::background_region_id,
                                                                assembly);
            std::vector<std::vector<subdomain_id_type>> duct_region_ids =
                getMeshProperty<std::vector<std::vector<subdomain_id_type>>>(RGMB::duct_region_ids,
                                                                             assembly);
            _background_region_id_map.insert(
                std::pair<subdomain_id_type, std::vector<subdomain_id_type>>(
                    assembly_type, background_region_ids));
            _duct_region_id_map.insert(
                std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
                    assembly_type, duct_region_ids));

            std::vector<std::string> background_block_names =
                getMeshProperty<std::vector<std::string>>(RGMB::background_block_name, assembly);
            std::vector<std::vector<std::string>> duct_block_names =
                getMeshProperty<std::vector<std::vector<std::string>>>(RGMB::duct_block_names,
                                                                       assembly);
            _background_block_name_map.insert(
                std::pair<subdomain_id_type, std::vector<std::string>>(assembly_type,
                                                                       background_block_names));
            _duct_block_name_map.insert(
                std::pair<subdomain_id_type, std::vector<std::vector<std::string>>>(
                    assembly_type, duct_block_names));
          }
        }
        else
        {
          // For control drum structures, store region ID and block name information of drum regions
          const auto & drum_region_ids =
              getMeshProperty<std::vector<std::vector<subdomain_id_type>>>(RGMB::drum_region_ids,
                                                                           assembly);
          _drum_region_id_map.insert(
              std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
                  assembly_type, drum_region_ids));
          std::vector<std::vector<std::string>> drum_block_names =
              getMeshProperty<std::vector<std::vector<std::string>>>(RGMB::drum_block_names,
                                                                     assembly);
          _drum_block_name_map.insert(
              std::pair<subdomain_id_type, std::vector<std::vector<std::string>>>(
                  assembly_type, drum_block_names));
        }
      }
    }

    // periphery meshing
    if (_mesh_periphery)
    {
      std::string periphery_mg_name = (_periphery_meshgenerator == "triangle")
                                          ? "PeripheralTriangleMeshGenerator"
                                          : "PeripheralRingMeshGenerator";

      // set up common options
      auto params = _app.getFactory().getValidParams(periphery_mg_name);
      params.set<MeshGeneratorName>("input") = name() + "_delbds";
      params.set<Real>("peripheral_ring_radius") = _outer_circle_radius;
      params.set<BoundaryName>("external_boundary_name") = "outside_periphery";
      params.set<SubdomainName>("peripheral_ring_block_name") = RGMB::PERIPHERAL_RING_BLOCK_NAME;

      // unique MG options
      if (_periphery_meshgenerator == "triangle")
      {
        params.set<unsigned int>("peripheral_ring_num_segments") = _outer_circle_num_segments;
        params.set<Real>("desired_area") = _desired_area;
        params.set<std::string>("desired_area_func") = _desired_area_func;
      }
      else if (_periphery_meshgenerator == "quad_ring")
      {
        params.set<subdomain_id_type>("peripheral_ring_block_id") = RGMB::PERIPHERAL_RING_BLOCK_ID;
        params.set<BoundaryName>("input_mesh_external_boundary") = RGMB::CORE_BOUNDARY_NAME;
        params.set<unsigned int>("peripheral_layer_num") = _periphery_num_layers;
      }

      // finish periphery input
      build_mesh_name = name() + "_periphery";
      addMeshSubgenerator(periphery_mg_name, build_mesh_name, params);
    }

    if (_extrude && _mesh_dimensions == 3)
      build_mesh_name = callExtrusionMeshSubgenerators(build_mesh_name);

    // Store final mesh subgenerator
    _build_mesh = &getMeshByName(build_mesh_name);
  }
  // If mesh generation should be bypassed, call getMeshes to resolve MeshGeneratorSystem
  // dependencies
  else
    auto input_meshes = getMeshes("inputs");

  generateMetadata();
}

void
CoreMeshGenerator::generateMetadata()
{
  // Define metadata related to downstream function calls
  if (_mesh_periphery)
  {
    declareMeshProperty(RGMB::peripheral_ring_radius, _outer_circle_radius);
    declareMeshProperty(RGMB::peripheral_ring_region_id, _periphery_region_id);
  }

  // Determine constituent pin type ids and define lattice
  std::vector<std::vector<int>> assembly_name_lattice;
  std::vector<std::string> input_assembly_names;
  std::vector<std::string> input_pin_names;

  // Iterate through input assembly names and define constituent assemblies and pins
  for (const auto i : index_range(_inputs))
  {
    const auto input_assembly_name = _inputs[i];
    if (input_assembly_name != _empty_key)
    {
      input_assembly_names.push_back(input_assembly_name);
      if (!getMeshProperty<bool>(RGMB::is_control_drum, input_assembly_name) &&
          !getMeshProperty<bool>(RGMB::is_single_pin, input_assembly_name))
      {
        const auto pin_names =
            getMeshProperty<std::vector<std::string>>(RGMB::pin_names, input_assembly_name);
        for (const auto & pin_name : pin_names)
          if (std::find(input_pin_names.begin(), input_pin_names.end(), pin_name) ==
              input_pin_names.end())
            input_pin_names.push_back(pin_name);
      }
    }
  }

  // Iterate through pattern and remap dummy assemblies with index -1
  for (const auto i : index_range(_pattern))
  {
    std::vector<int> assembly_name_idx(_pattern[i].size());
    for (const auto j : index_range(_pattern[i]))
    {
      const auto input_assembly_name = _inputs[_pattern[i][j]];
      // Use an assembly type of -1 to represent a dummy assembly
      if (input_assembly_name == _empty_key)
        assembly_name_idx[j] = -1;
      // Set index of assembly name based on `input_assembly_names` variable
      else
      {
        const auto it = std::find(
            input_assembly_names.begin(), input_assembly_names.end(), input_assembly_name);
        assembly_name_idx[j] = it - input_assembly_names.begin();
      }
    }
    assembly_name_lattice.push_back(assembly_name_idx);
  }

  declareMeshProperty(RGMB::pin_names, input_pin_names);
  declareMeshProperty(RGMB::assembly_names, input_assembly_names);
  declareMeshProperty(RGMB::assembly_lattice, assembly_name_lattice);
  declareMeshProperty(RGMB::extruded, _extrude && _mesh_dimensions == 3);
}

bool
CoreMeshGenerator::constituentAssembliesNeedFlexibleStiching()
{
  MeshGeneratorName first_nondummy_assembly = "";
  bool assembly_homogenization = false;
  unsigned int n_constituent_pins = 0;
  unsigned int n_pin_sectors = 0;

  // Loop through all non-dummy input assemblies. Flexible assembly stitching is needed if one of
  // the following criteria are met:
  // 1. The number of constituent pins within the assembly does not match with another assembly
  // 2. The value of is_single_pin and is_homogenized metadata do not agree with another assembly
  // 3. The number of sectors of the constituent pins of an assembly do not match with the
  // constituent pins of another assembly
  for (const auto i : index_range(_inputs))
  {
    // Skip if assembly name is equal to dummy assembly name
    if (_inputs[i] == _empty_key)
      continue;

    // Compute total number of constituent pins in assembly, as well as the number of sectors per
    // side for each pin Note: number of sectors per side is defined uniformly across constituent
    // pins of an assembly, so only first one needs to be checked
    unsigned int total_pins = 0;
    unsigned int pin_sectors_per_side = 0;
    if (!getMeshProperty<bool>(RGMB::is_single_pin, _inputs[i]))
    {
      const auto first_pin_name =
          getMeshProperty<std::vector<std::string>>(RGMB::pin_names, _inputs[i])[0];
      pin_sectors_per_side = getMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta",
                                                                        first_pin_name + "_2D")[0];
      const auto pin_lattice =
          getMeshProperty<std::vector<std::vector<int>>>(RGMB::pin_lattice, _inputs[i]);
      for (const auto i : index_range(pin_lattice))
        total_pins += pin_lattice[i].size();
    }
    else
    {
      if (getMeshProperty<bool>(RGMB::is_homogenized, _inputs[i]))
      {
        // Homogenized assembly
        total_pins = 0;
        pin_sectors_per_side = 0;
      }
      else
      {
        // Assembly with single constituent pin
        total_pins = 1;
        pin_sectors_per_side = getMeshProperty<std::vector<unsigned int>>(
            "num_sectors_per_side_meta", _inputs[i] + "_2D")[0];
      }
    }

    if (first_nondummy_assembly == "")
    {
      first_nondummy_assembly = MeshGeneratorName(_inputs[i]);
      assembly_homogenization = getMeshProperty<bool>(RGMB::is_homogenized, _inputs[i]);
      n_constituent_pins = total_pins;
      n_pin_sectors = pin_sectors_per_side;
    }
    else
    {
      if (getMeshProperty<bool>(RGMB::is_homogenized, _inputs[i]) != assembly_homogenization)
      {
        mooseWarning("Detected mix of homogenized and heterogeneous assemblies between " +
                     first_nondummy_assembly + " and " + _inputs[i]);
        return true;
      }
      if (total_pins != n_constituent_pins)
      {
        mooseWarning(
            "Detected assemblies with different number of total constituent pins between " +
            first_nondummy_assembly + " and " + _inputs[i]);
        return true;
      }
      if (pin_sectors_per_side != n_pin_sectors)
      {
        mooseWarning("Constituent pins in " + first_nondummy_assembly + " and " + _inputs[i] +
                     " differ in terms of number of sectors per side");
        return true;
      }
    }
  }
  return false;
}

std::unique_ptr<MeshBase>
CoreMeshGenerator::generate()
{
  // Must be called to free the ReactorMeshParams mesh
  freeReactorMeshParams();

  // If bypass_mesh is true, return a null mesh. In this mode, an output mesh is not
  // generated and only metadata is defined on the generator, so logic related to
  // generation of output mesh will not be called
  if (getReactorParam<bool>(RGMB::bypass_meshgen))
  {
    auto null_mesh = nullptr;
    return null_mesh;
  }
  // This generate() method will be called once the subgenerators that we depend on are
  // called. This is where we reassign subdomain ids/names in case they were merged
  // when stitching assemblies into the core. This is also where we set region_id extra
  // element integers, which has not been set yet for extruded geometries

  // Define all extra element names and integers
  std::string pin_type_id_name = "pin_type_id";
  std::string assembly_type_id_name = "assembly_type_id";
  std::string plane_id_name = "plane_id";
  std::string region_id_name = "region_id";
  std::string radial_id_name = "radial_id";
  const std::string default_block_name = RGMB::CORE_BLOCK_NAME_PREFIX;

  auto pin_type_id_int = getElemIntegerFromMesh(*(*_build_mesh), pin_type_id_name, true);
  auto assembly_type_id_int = getElemIntegerFromMesh(*(*_build_mesh), assembly_type_id_name, true);
  auto radial_id_int = getElemIntegerFromMesh(*(*_build_mesh), radial_id_name, true);
  auto region_id_int = getElemIntegerFromMesh(*(*_build_mesh), region_id_name, true);
  unsigned int plane_id_int = 0;
  if (_extrude)
    plane_id_int = getElemIntegerFromMesh(*(*_build_mesh), plane_id_name, true);

  // Get next free block ID in mesh in case subdomain ids need to be remapped
  auto next_block_id = MooseMeshUtils::getNextFreeSubdomainID(*(*(_build_mesh)));
  std::map<std::string, SubdomainID> rgmb_name_id_map;

  // Loop through all mesh elements and set region ids and reassign block IDs/names
  // if they were merged during assembly stitching
  for (auto & elem : (*_build_mesh)->active_element_ptr_range())
  {
    dof_id_type z_id = _extrude ? elem->get_extra_integer(plane_id_int) : 0;
    dof_id_type pin_type_id = elem->get_extra_integer(pin_type_id_int);

    if (_pin_region_id_map.find(pin_type_id) != _pin_region_id_map.end())
    {
      // Pin type element, get region ID from pin_type, z_id, and radial_idx
      const dof_id_type radial_idx = elem->get_extra_integer(radial_id_int);
      const auto elem_rid = _pin_region_id_map[pin_type_id][z_id][radial_idx];
      elem->set_extra_integer(region_id_int, elem_rid);

      // Set element block name and block id
      bool has_block_names = !_pin_block_name_map[pin_type_id].empty();
      auto elem_block_name = default_block_name;
      if (has_block_names)
        elem_block_name += "_" + _pin_block_name_map[pin_type_id][z_id][radial_idx];
      else if (getReactorParam<bool>(RGMB::region_id_as_block_name))
        elem_block_name += "_REG" + std::to_string(elem_rid);
      if (elem->type() == TRI3 || elem->type() == PRISM6)
        elem_block_name += RGMB::TRI_BLOCK_NAME_SUFFIX;
      updateElementBlockNameId(
          *(*_build_mesh), elem, rgmb_name_id_map, elem_block_name, next_block_id);
    }
    else if ((*_build_mesh)->subdomain_name(elem->subdomain_id()) ==
             RGMB::PERIPHERAL_RING_BLOCK_NAME)
    // periphery type element
    {
      // set region ID of core periphery element
      elem->set_extra_integer(region_id_int, _periphery_region_id);
      // set block name and block name of core periphery element
      auto elem_block_name = _periphery_block_name;
      if (getReactorParam<bool>(RGMB::region_id_as_block_name))
        elem_block_name += "_REG" + std::to_string(_periphery_region_id);
      if (elem->type() == TRI3 || elem->type() == PRISM6)
        elem_block_name += RGMB::TRI_BLOCK_NAME_SUFFIX;
      updateElementBlockNameId(
          *(*_build_mesh), elem, rgmb_name_id_map, elem_block_name, next_block_id);
    }
    else
    {
      dof_id_type assembly_type_id = elem->get_extra_integer(assembly_type_id_int);
      // Infer peripheral index of assembly background, assembly duct, or control drum regions from
      // pin_type_id
      unsigned int peripheral_idx = RGMB::MAX_PIN_TYPE_ID - pin_type_id;

      // check if element is part of drum region
      if (_drum_region_id_map.find(assembly_type_id) != _drum_region_id_map.end())
      {
        // Element is in a control drum region. Infer region id from assembly_type_id, z_id, and
        // peripheral_index
        const auto elem_rid = _drum_region_id_map[assembly_type_id][z_id][peripheral_idx];
        elem->set_extra_integer(region_id_int, elem_rid);

        // Set element block name and block id
        auto elem_block_name = default_block_name;
        if (getReactorParam<bool>(RGMB::region_id_as_block_name))
          elem_block_name += "_REG" + std::to_string(elem_rid);
        else
        {
          bool has_drum_block_name = !_drum_block_name_map[assembly_type_id].empty();
          if (has_drum_block_name)
            elem_block_name += "_" + _drum_block_name_map[assembly_type_id][z_id][peripheral_idx];
        }
        if (elem->type() == TRI3 || elem->type() == PRISM6)
          elem_block_name += RGMB::TRI_BLOCK_NAME_SUFFIX;
        updateElementBlockNameId(
            *(*_build_mesh), elem, rgmb_name_id_map, elem_block_name, next_block_id);
      }
      else
      {
        // Element is in an assembly duct or background region since it doesn't
        // have an assembly type id in the drum region map. Infer region id from
        // assembly_type_id, z_id, and peripheral_index
        bool is_background_region = peripheral_idx == 0;
        const auto elem_rid =
            (is_background_region
                 ? _background_region_id_map[assembly_type_id][z_id]
                 : _duct_region_id_map[assembly_type_id][z_id][peripheral_idx - 1]);
        elem->set_extra_integer(region_id_int, elem_rid);

        // Set element block name and block id
        auto elem_block_name = default_block_name;
        if (getReactorParam<bool>(RGMB::region_id_as_block_name))
          elem_block_name += "_REG" + std::to_string(elem_rid);
        else
        {
          if (is_background_region)
          {
            bool has_background_block_name = !_background_block_name_map[assembly_type_id].empty();
            if (has_background_block_name)
              elem_block_name += "_" + _background_block_name_map[assembly_type_id][z_id];
          }
          else
          {
            bool has_duct_block_names = !_duct_block_name_map[assembly_type_id].empty();
            if (has_duct_block_names)
              elem_block_name +=
                  "_" + _duct_block_name_map[assembly_type_id][z_id][peripheral_idx - 1];
          }
        }
        if (elem->type() == TRI3 || elem->type() == PRISM6)
          elem_block_name += RGMB::TRI_BLOCK_NAME_SUFFIX;
        updateElementBlockNameId(
            *(*_build_mesh), elem, rgmb_name_id_map, elem_block_name, next_block_id);
      }
    }
  }

  // Sideset 10000 does not get stitched properly when BlockDeletionGenerator
  // is used for deleting dummy assemblies. This block copies missing sides
  // into sideset 10000 from sideset RGMB::CORE_BOUNDARY_NAME
  BoundaryInfo & boundary_info = (*_build_mesh)->get_boundary_info();
  boundary_id_type source_id =
      MooseMeshUtils::getBoundaryIDs(**_build_mesh, {RGMB::CORE_BOUNDARY_NAME}, true)[0];
  boundary_id_type target_id = 10000;
  const auto sideset_map = boundary_info.get_sideset_map();

  for (const auto & [elem, id_pair] : sideset_map)
  {
    const auto side_id = id_pair.first;
    const auto sideset_id = id_pair.second;

    // Filter all sides that belong to RGMB::CORE_BOUNDARY_NAME sideset
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

  if (getParam<bool>("generate_depletion_id"))
  {
    const MooseEnum option = getParam<MooseEnum>("depletion_id_type");
    addDepletionId(*(*_build_mesh), option, DepletionIDGenerationLevel::Core, _extrude);
  }

  // Mark mesh as not prepared, as block ID's were re-assigned in this method
  (*_build_mesh)->set_isnt_prepared();

  return std::move(*_build_mesh);
}
