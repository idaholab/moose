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
      "inputs", "The AssemblyMeshGenerators that form the components of the assembly.");

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
      "periphery_block_name", "RGMB_CORE", "Block name for periphery zone.");
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

  // Declare that all of the meshes in the "inputs" parameter are to be used by
  // a sub mesh generator
  declareMeshesForSub("inputs");

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
    if (_periphery_meshgenerator == "triangle")
    {
      if (parameters.isParamSetByUser("periphery_num_layers"))
      {
        paramError("periphery_num_layers",
                   "periphery_num_layers cannot be used with PTMG periphery mesher.");
      }
    }
  }

  MeshGeneratorName reactor_params =
      MeshGeneratorName(getMeshProperty<std::string>("reactor_params_name", _inputs[0]));
  const auto assembly_homogenization = getMeshProperty<bool>("homogenized_assembly", _inputs[0]);
  // Check that MG name for reactor params and assembly homogenization schemes are
  // consistent across all assemblies
  for (unsigned int i = 1; i < _inputs.size(); i++)
  {
    // Skip if assembly name is equal to dummy assembly name
    if (_inputs[i] == _empty_key)
      continue;
    if (getMeshProperty<std::string>("reactor_params_name", _inputs[i]) != reactor_params)
      mooseError("The name of all reactor_params objects should be identical across all pins in "
                 "the input assemblies.\n");
    if (getMeshProperty<bool>("homogenized_assembly", _inputs[i]) != assembly_homogenization)
      mooseError(
          "All assemblies in the core must be homogenized if assembly homogenization is used\n");
  }

  // Initialize ReactorMeshParams object stored in pin input
  initializeReactorMeshParams(reactor_params);

  _geom_type = getReactorParam<std::string>("mesh_geometry");
  _mesh_dimensions = getReactorParam<int>("mesh_dimensions");

  if (_extrude && _mesh_dimensions != 3)
    mooseError("This is a 2 dimensional mesh, you cannot extrude it. Check your ReactorMeshParams "
               "inputs\n");
  if (_extrude && (!hasReactorParam<boundary_id_type>("top_boundary_id") ||
                   !hasReactorParam<boundary_id_type>("bottom_boundary_id")))
    mooseError("Both top_boundary_id and bottom_boundary_id must be provided in ReactorMeshParams "
               "if using extruded geometry");
  if (!hasReactorParam<boundary_id_type>("radial_boundary_id"))
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

  if (_geom_type == "Square")
  {
    // create a dummy assembly that is 1 assembly sized element that will get deleted later
    if (make_empty)
    {
      Real pitch = getReactorParam<Real>("assembly_pitch");

      auto params = _app.getFactory().getValidParams("PolygonConcentricCircleMeshGenerator");
      params.set<unsigned int>("num_sides") = 4;
      params.set<std::vector<unsigned int>>("num_sectors_per_side") =
          std::vector<unsigned int>(4, 2);
      params.set<Real>("polygon_size") = pitch / 2.0;
      params.set<std::vector<subdomain_id_type>>("background_block_ids") =
          std::vector<subdomain_id_type>{UINT16_MAX - 1};
      params.set<bool>("flat_side_up") = true;

      addMeshSubgenerator("PolygonConcentricCircleMeshGenerator", std::string(_empty_key), params);
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

      addMeshSubgenerator("SideSetsFromNormalsGenerator", name() + "_bds", params);
    }
    {
      auto params = _app.getFactory().getValidParams("RenameBoundaryGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_bds";
      params.set<std::vector<BoundaryName>>("old_boundary") = {
          "tmp_left", "tmp_right", "tmp_top", "tmp_bottom"};
      params.set<std::vector<BoundaryName>>("new_boundary") =
          std::vector<BoundaryName>(4, "outer_core");

      addMeshSubgenerator("RenameBoundaryGenerator", name() + "_pattern", params);
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
        if (assembly_homogenization)
        {
          auto params = _app.getFactory().getValidParams("SimpleHexagonGenerator");

          params.set<Real>("hexagon_size") = getReactorParam<Real>("assembly_pitch") / 2.0;
          params.set<std::vector<subdomain_id_type>>("block_id") = {UINT16_MAX - 1};

          addMeshSubgenerator("SimpleHexagonGenerator", std::string(_empty_key), params);
        }
        else
        {
          auto params = _app.getFactory().getValidParams(
              "HexagonConcentricCircleAdaptiveBoundaryMeshGenerator");

          params.set<Real>("hexagon_size") = getReactorParam<Real>("assembly_pitch") / 2.0;
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
    }
    {
      auto params = _app.getFactory().getValidParams("HexIDPatternedMeshGenerator");

      params.set<std::string>("id_name") = "assembly_id";
      params.set<MooseEnum>("assign_type") =
          "cell"; // give elems IDs relative to position in assembly
      params.set<std::vector<MeshGeneratorName>>("inputs") = _inputs;
      params.set<std::vector<std::vector<unsigned int>>>("pattern") = _pattern;
      params.set<MooseEnum>("pattern_boundary") = "none";
      params.set<bool>("generate_core_metadata") = !assembly_homogenization;
      params.set<bool>("create_outward_interface_boundaries") = false;
      if (make_empty)
      {
        params.set<std::vector<MeshGeneratorName>>("exclude_id") =
            std::vector<MeshGeneratorName>{_empty_key};
      }

      const auto radial_boundary = getReactorParam<boundary_id_type>("radial_boundary_id");
      params.set<boundary_id_type>("external_boundary_id") = radial_boundary;
      params.set<std::string>("external_boundary_name") = "outer_core";

      addMeshSubgenerator("HexIDPatternedMeshGenerator", name() + "_pattern", params);
    }
  }
  if (_empty_pos)
  {
    auto params = _app.getFactory().getValidParams("BlockDeletionGenerator");

    params.set<SubdomainID>("block_id") = UINT16_MAX - 1;
    params.set<MeshGeneratorName>("input") = name() + "_pattern";
    params.set<BoundaryName>("new_boundary") = "outer_core";

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
        const auto assembly_id = getMeshProperty<subdomain_id_type>("assembly_type", assembly_name);
        const BoundaryName boundary_name = "outer_assembly_" + std::to_string(assembly_id);
        if (!std::count(boundaries_to_delete.begin(), boundaries_to_delete.end(), boundary_name))
          boundaries_to_delete.push_back(boundary_name);
      }
    }
    auto params = _app.getFactory().getValidParams("BoundaryDeletionGenerator");

    params.set<MeshGeneratorName>("input") = _empty_pos ? name() + "_deleted" : name() + "_pattern";
    params.set<std::vector<BoundaryName>>("boundary_names") = boundaries_to_delete;

    build_mesh_name = name() + "_delbds";
    addMeshSubgenerator("BoundaryDeletionGenerator", build_mesh_name, params);
  }

  for (auto assembly : _inputs)
  {
    if (assembly != _empty_key)
    {
      std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>> pin_region_id_map =
          getMeshProperty<std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>>(
              "pin_region_id_map", assembly);
      for (auto pin = pin_region_id_map.begin(); pin != pin_region_id_map.end(); ++pin)
      {
        if (_pin_region_id_map.find(pin->first) == _pin_region_id_map.end())
          _pin_region_id_map.insert(
              std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
                  pin->first, pin->second));
        else if (pin->second != _pin_region_id_map.find(pin->first)->second)
          mooseError("Multiple region id definitions for the same pin type. Check pin_type ids.\n");
      }
      std::map<subdomain_id_type, std::vector<std::vector<std::string>>> pin_block_name_map =
          getMeshProperty<std::map<subdomain_id_type, std::vector<std::vector<std::string>>>>(
              "pin_block_name_map", assembly);
      for (auto pin = pin_block_name_map.begin(); pin != pin_block_name_map.end(); ++pin)
      {
        if (_pin_block_name_map.find(pin->first) == _pin_block_name_map.end())
          _pin_block_name_map.insert(
              std::pair<subdomain_id_type, std::vector<std::vector<std::string>>>(pin->first,
                                                                                  pin->second));
        else if (pin->second != _pin_block_name_map.find(pin->first)->second)
          mooseError(
              "Multiple block name definitions for the same pin type. Check pin_type names.\n");
      }

      if (_geom_type == "Hex")
      {
        subdomain_id_type assembly_type =
            getMeshProperty<subdomain_id_type>("assembly_type", assembly);
        if (_background_region_id_map.find(assembly_type) == _background_region_id_map.end())
        {
          // Store region ids and block names associated with duct and background regions for each
          // assembly, in case block names need to be recovered from region ids after
          // multiple assemblies have been stitched together into a core
          std::vector<subdomain_id_type> background_region_ids =
              getMeshProperty<std::vector<subdomain_id_type>>("background_region_ids", assembly);
          std::vector<std::vector<subdomain_id_type>> duct_region_ids =
              getMeshProperty<std::vector<std::vector<subdomain_id_type>>>("duct_region_ids",
                                                                           assembly);
          _background_region_id_map.insert(
              std::pair<subdomain_id_type, std::vector<subdomain_id_type>>(assembly_type,
                                                                           background_region_ids));
          _duct_region_id_map.insert(
              std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
                  assembly_type, duct_region_ids));

          std::vector<std::string> background_block_names =
              getMeshProperty<std::vector<std::string>>("background_block_names", assembly);
          std::vector<std::vector<std::string>> duct_block_names =
              getMeshProperty<std::vector<std::vector<std::string>>>("duct_block_names", assembly);
          _background_block_name_map.insert(std::pair<subdomain_id_type, std::vector<std::string>>(
              assembly_type, background_block_names));
          _duct_block_name_map.insert(
              std::pair<subdomain_id_type, std::vector<std::vector<std::string>>>(
                  assembly_type, duct_block_names));
        }
      }
    }
  }
  declareMeshProperty("pin_id_map", _pin_region_id_map);

  declareMeshProperty("assembly_pitch", getReactorParam<Real>("assembly_pitch"));

  // periphery meshing
  if (_mesh_periphery)
  {
    std::string periphery_mg_name;
    if (_periphery_meshgenerator == "triangle")
      periphery_mg_name = "PeripheralTriangleMeshGenerator";
    else if (_periphery_meshgenerator == "quad_ring")
      periphery_mg_name = "PeripheralRingMeshGenerator";
    else
      paramError("periphery_generator",
                 "Provided periphery meshgenerator has not been implemented.");

    // set up common options
    auto params = _app.getFactory().getValidParams(periphery_mg_name);
    params.set<MeshGeneratorName>("input") = name() + "_delbds";
    params.set<Real>("peripheral_ring_radius") = _outer_circle_radius;
    params.set<std::string>("external_boundary_name") = "outside_periphery";
    params.set<SubdomainName>("peripheral_ring_block_name") = "RGMB_PERIPHERY_GENERATED";

    // unique MG options
    if (_periphery_meshgenerator == "triangle")
    {
      params.set<unsigned int>("peripheral_ring_num_segments") = _outer_circle_num_segments;
      params.set<Real>("desired_area") = _desired_area;
      params.set<std::string>("desired_area_func") = _desired_area_func;
    }
    else if (_periphery_meshgenerator == "quad_ring")
    {
      params.set<subdomain_id_type>("peripheral_ring_block_id") = 25000;
      params.set<BoundaryName>("input_mesh_external_boundary") = (BoundaryName) "outer_core";
      params.set<unsigned int>("peripheral_layer_num") = _periphery_num_layers;
    }

    // finish periphery input
    build_mesh_name = name() + "_periphery";
    addMeshSubgenerator(periphery_mg_name, build_mesh_name, params);
  }

  if (_extrude && _mesh_dimensions == 3)
  {
    std::vector<Real> axial_boundaries = getReactorParam<std::vector<Real>>("axial_boundaries");
    const auto top_boundary = getReactorParam<boundary_id_type>("top_boundary_id");
    const auto bottom_boundary = getReactorParam<boundary_id_type>("bottom_boundary_id");
    {
      declareMeshProperty("extruded", true);
      auto params = _app.getFactory().getValidParams("AdvancedExtruderGenerator");

      if (_mesh_periphery)
        params.set<MeshGeneratorName>("input") = name() + "_periphery";
      else
        params.set<MeshGeneratorName>("input") = name() + "_delbds";

      params.set<Point>("direction") = Point(0, 0, 1);
      params.set<std::vector<unsigned int>>("num_layers") =
          getReactorParam<std::vector<unsigned int>>("axial_mesh_intervals");
      params.set<std::vector<Real>>("heights") = axial_boundaries;
      params.set<boundary_id_type>("bottom_boundary") = bottom_boundary;
      params.set<boundary_id_type>("top_boundary") = top_boundary;

      addMeshSubgenerator("AdvancedExtruderGenerator", name() + "_extruded", params);
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

      build_mesh_name = name() + "_extrudedIDs";
      addMeshSubgenerator("PlaneIDMeshGenerator", build_mesh_name, params);
    }
  }
  else
    declareMeshProperty("extruded", false);

  _build_mesh = &getMeshByName(build_mesh_name);
}

std::unique_ptr<MeshBase>
CoreMeshGenerator::generate()
{
  // Must be called to free the ReactorMeshParams mesh
  freeReactorMeshParams();

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
  const std::string default_block_name = "RGMB_CORE";

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
      if (elem->type() == TRI3 || elem->type() == PRISM6)
        elem_block_name += "_TRI";
      updateElementBlockNameId(
          *(*_build_mesh), elem, rgmb_name_id_map, elem_block_name, next_block_id);
    }
    else if ((*_build_mesh)->subdomain_name(elem->subdomain_id()) == "RGMB_PERIPHERY_GENERATED")
    // periphery type element
    {
      // set region ID of core periphery element
      elem->set_extra_integer(region_id_int, _periphery_region_id);
      // set block name and block name of core periphery element
      auto elem_block_name = _periphery_block_name;
      if (elem->type() == TRI3 || elem->type() == PRISM6)
        elem_block_name += "_TRI";
      updateElementBlockNameId(
          *(*_build_mesh), elem, rgmb_name_id_map, elem_block_name, next_block_id);
    }
    else
    {
      // element is in an assembly duct or background region since it doesn't
      // have a pin type id that matches one in the map. Infer peripheral index
      // from pin_type and region id from assembly_type_id, z_id, and peripheral_index
      dof_id_type assembly_type_id = elem->get_extra_integer(assembly_type_id_int);
      unsigned int peripheral_idx = (UINT16_MAX - 1) - pin_type_id;
      bool is_background_region = peripheral_idx == 0;
      const auto elem_rid =
          (is_background_region ? _background_region_id_map[assembly_type_id][z_id]
                                : _duct_region_id_map[assembly_type_id][z_id][peripheral_idx - 1]);
      elem->set_extra_integer(region_id_int, elem_rid);

      // Set element block name and block id
      auto elem_block_name = default_block_name;
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
          elem_block_name += "_" + _duct_block_name_map[assembly_type_id][z_id][peripheral_idx - 1];
      }
      if (elem->type() == TRI3 || elem->type() == PRISM6)
        elem_block_name += "_TRI";
      updateElementBlockNameId(
          *(*_build_mesh), elem, rgmb_name_id_map, elem_block_name, next_block_id);
    }
  }

  // Sideset 10000 does not get stitched properly when BlockDeletionGenerator
  // is used for deleting dummy assemblies. This block copies missing sides
  // into sideset 10000 from sideset "outer_core"
  BoundaryInfo & boundary_info = (*_build_mesh)->get_boundary_info();
  boundary_id_type source_id =
      MooseMeshUtils::getBoundaryIDs(**_build_mesh, {"outer_core"}, true)[0];
  boundary_id_type target_id = 10000;
  const auto sideset_map = boundary_info.get_sideset_map();

  for (const auto & [elem, id_pair] : sideset_map)
  {
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

  (*_build_mesh)->find_neighbors();

  return std::move(*_build_mesh);
}
