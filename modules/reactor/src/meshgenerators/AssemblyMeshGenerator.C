//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AssemblyMeshGenerator.h"

#include "ReactorGeometryMeshBuilderBase.h"
#include "MooseApp.h"
#include "Factory.h"
#include "libmesh/elem.h"
#include "MooseMeshUtils.h"

registerMooseObject("ReactorApp", AssemblyMeshGenerator);

InputParameters
AssemblyMeshGenerator::validParams()
{
  auto params = ReactorGeometryMeshBuilderBase::validParams();

  params.addRequiredParam<std::vector<MeshGeneratorName>>(
      "inputs", "The PinMeshGenerators that form the components of the assembly.");

  params.addRequiredParam<subdomain_id_type>("assembly_type",
                                             "The integer ID for this assembly type definition");

  params.addRequiredParam<std::vector<std::vector<unsigned int>>>(
      "pattern",
      "A double-indexed array starting with the upper-left corner where the index"
      "represents the layout of input pins in the assembly lattice.");

  params.addRangeCheckedParam<std::vector<Real>>(
      "duct_halfpitch",
      "duct_halfpitch>0.0",
      "Distance(s) from center to duct(s) inner boundaries.");

  params.addRangeCheckedParam<unsigned int>("background_intervals",
                                            "background_intervals>0",
                                            "Radial intervals in the assembly peripheral region.");

  params.addRangeCheckedParam<std::vector<unsigned int>>(
      "duct_intervals", "duct_intervals>0", "Number of meshing intervals in each enclosing duct.");

  params.addParam<std::vector<subdomain_id_type>>(
      "background_region_id",
      "The region id for the background area between the pins and the ducts to set region_id "
      "extra-element integer");

  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "duct_region_ids",
      "The region id for the ducts from innermost to outermost, to set region_id "
      "extra-element integer.");

  params.addParam<std::vector<std::string>>("background_block_name",
                                            "The block names for the assembly background regions");

  params.addParam<std::vector<std::vector<std::string>>>(
      "duct_block_names",
      "The block names for the assembly duct regions from innermost to outermost");

  params.addParam<bool>("extrude",
                        false,
                        "Determines if this is the final step in the geometry construction"
                        " and extrudes the 2D geometry to 3D. If this is true then this mesh "
                        "cannot be used in further mesh building in the Reactor workflow");
  params.addParamNamesToGroup("background_region_id duct_region_ids assembly_type", "ID assigment");
  params.addParamNamesToGroup("background_intervals background_region_id",
                              "Background specifications");
  params.addParamNamesToGroup("duct_intervals duct_region_ids duct_halfpitch",
                              "Duct specifications");

  params.addClassDescription("This AssemblyMeshGenerator object is designed to generate "
                             "assembly-like structures, with IDs, from a reactor geometry. "
                             "The assembly-like structures must consist of a full pattern of equal "
                             "sized pins from PinMeshGenerator"
                             "A hexagonal assembly will be placed inside of a bounding hexagon "
                             "consisting of a background region and, optionally,"
                             " duct regions.");

  return params;
}

AssemblyMeshGenerator::AssemblyMeshGenerator(const InputParameters & parameters)
  : ReactorGeometryMeshBuilderBase(parameters),
    _inputs(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _assembly_type(getParam<subdomain_id_type>("assembly_type")),
    _pattern(getParam<std::vector<std::vector<unsigned int>>>("pattern")),
    _duct_sizes(isParamValid("duct_halfpitch") ? getParam<std::vector<Real>>("duct_halfpitch")
                                               : std::vector<Real>()),
    _background_intervals(
        isParamValid("background_intervals") ? getParam<unsigned int>("background_intervals") : 0),
    _duct_intervals(isParamValid("duct_intervals")
                        ? getParam<std::vector<unsigned int>>("duct_intervals")
                        : std::vector<unsigned int>()),
    _background_region_id(isParamValid("background_region_id")
                              ? getParam<std::vector<subdomain_id_type>>("background_region_id")
                              : std::vector<subdomain_id_type>()),
    _duct_region_ids(isParamValid("duct_region_ids")
                         ? getParam<std::vector<std::vector<subdomain_id_type>>>("duct_region_ids")
                         : std::vector<std::vector<subdomain_id_type>>()),
    _extrude(getParam<bool>("extrude"))
{
  declareMeshesForSub("inputs");

  MeshGeneratorName reactor_params =
      MeshGeneratorName(getMeshProperty<std::string>("reactor_params_name", _inputs[0]));
  // Check that MG name for reactor params is consistent across all assemblies
  for (unsigned int i = 1; i < _inputs.size(); i++)
    if (getMeshProperty<std::string>("reactor_params_name", _inputs[i]) != reactor_params)
      mooseError("The name of all reactor_params objects should be identical across all input pins "
                 "in the assembly.\n");

  // Initialize ReactorMeshParams object stored in pin input
  initializeReactorMeshParams(reactor_params);

  _geom_type = getReactorParam<std::string>("mesh_geometry");
  _mesh_dimensions = getReactorParam<int>("mesh_dimensions");
  declareMeshProperty("assembly_type", _assembly_type);
  declareMeshProperty("homogenized_assembly", false);

  if (_extrude && _mesh_dimensions != 3)
    paramError("extrude",
               "This is a 2 dimensional mesh, you cannot extrude it. Check you ReactorMeshParams "
               "inputs\n");
  if (_extrude && (!hasReactorParam<boundary_id_type>("top_boundary_id") ||
                   !hasReactorParam<boundary_id_type>("bottom_boundary_id")))
    mooseError("Both top_boundary_id and bottom_boundary_id must be provided in ReactorMeshParams "
               "if using extruded geometry");

  Real base_pitch = 0.0;
  for (const auto i : index_range(_inputs))
  {
    auto pin = _inputs[i];
    if (i == 0)
      base_pitch = getMeshProperty<Real>("pitch", pin);
    else
    {
      auto pitch = getMeshProperty<Real>("pitch", pin);
      if (!MooseUtils::absoluteFuzzyEqual(pitch, base_pitch))
        mooseError("All pins within an assembly must have the same pitch");
    }
    if (getMeshProperty<bool>("extruded", pin))
      mooseError("Pins that have already been extruded cannot be used in AssemblyMeshGenerator "
                 "definition.\n");
  }
  auto assembly_pitch = getReactorParam<Real>("assembly_pitch");
  if (_geom_type == "Square")
  {
    if (_duct_sizes.size() != 0 || _duct_intervals.size() != 0 || _background_intervals != 0)
      mooseError("Ducts and background regions are not currently supported for square assemblies");
    if ((!MooseUtils::absoluteFuzzyEqual(base_pitch * _pattern.size(), assembly_pitch)) ||
        (!MooseUtils::absoluteFuzzyEqual(base_pitch * _pattern[0].size(), assembly_pitch)))
      mooseError("Assembly pitch must be equal to lattice dimension times pin pitch for Cartesian "
                 "assemblies");
  }

  unsigned int n_axial_levels =
      (_mesh_dimensions == 3)
          ? getReactorParam<std::vector<unsigned int>>("axial_mesh_intervals").size()
          : 1;
  if (_geom_type == "Hex")
  {
    if ((_background_region_id.size() == 0) || _background_intervals == 0)
      mooseError("Hexagonal assemblies must have a background region defined");
    if (assembly_pitch / std::sin(M_PI / 3.0) < _pattern.size() * base_pitch)
      mooseError("Hexagonal diameter of assembly must be larger than the number of assembly rows "
                 "times the pin pitch");
    // Check size of background region id matches number of axial levels
    if (_background_region_id.size() != n_axial_levels)
      mooseError("The size of background_region_id must be equal to the number of axial levels as "
                 "defined in the ReactorMeshParams object");
  }

  if (_duct_sizes.size() != _duct_intervals.size())
    mooseError("If ducts are defined then \"duct_intervals\" and \"duct_region_ids\" must also be "
               "defined and of equal size.");

  if (_duct_sizes.size() != 0)
  {
    // Check size of duct region id matches number of axial levels
    if (_duct_region_ids.size() != n_axial_levels)
      mooseError("The size of duct_region_id must be equal to the number of axial levels as "
                 "defined in the ReactorMeshParams object");
    if (_duct_region_ids[0].size() != _duct_sizes.size())
      paramError("duct_halfpitch",
                 "If ducts are defined, then \"duct_intervals\" and \"duct_region_ids\" "
                 "must also be defined and of equal size.");
  }

  // Check whether block names are defined properly
  if (isParamValid("background_block_name"))
  {
    _has_background_block_name = true;
    _background_block_name = getParam<std::vector<std::string>>("background_block_name");
    if (_background_region_id.size() != _background_block_name.size())
      mooseError("The size of background_block_name must match the size of background_region_id");
  }
  else
    _has_background_block_name = false;

  if (isParamValid("duct_block_names"))
  {
    _has_duct_block_names = true;
    _duct_block_names = getParam<std::vector<std::vector<std::string>>>("duct_block_names");
    if (_duct_region_ids.size() != _duct_block_names.size())
      mooseError("The size of duct_block_names must match the size of duct_region_ids");
    for (const auto i : index_range(_duct_region_ids))
      if (_duct_region_ids[i].size() != _duct_block_names[i].size())
        mooseError("The size of duct_block_names must match the size of duct_region_ids");
  }
  else
    _has_duct_block_names = false;

  _assembly_boundary_id = 2000 + _assembly_type;
  _assembly_boundary_name = "outer_assembly_" + std::to_string(_assembly_type);

  if (_geom_type == "Square")
  {
    {
      auto params = _app.getFactory().getValidParams("CartesianIDPatternedMeshGenerator");

      params.set<std::string>("id_name") = "pin_id";
      params.set<MooseEnum>("assign_type") =
          "cell"; // give elems IDs relative to position in assembly
      params.set<std::vector<MeshGeneratorName>>("inputs") = _inputs;
      params.set<std::vector<std::vector<unsigned int>>>("pattern") = _pattern;
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
          std::vector<BoundaryName>(4, _assembly_boundary_name);

      addMeshSubgenerator("RenameBoundaryGenerator", name() + "_pattern", params);
    }
    //***Add assembly duct around PatternedMesh
  }
  else
  {
    // Hex Geometry
    {
      auto params = _app.getFactory().getValidParams("PatternedHexMeshGenerator");

      params.set<std::string>("id_name") = "pin_id";
      params.set<MooseEnum>("assign_type") =
          "cell"; // give elems IDs relative to position in assembly
      params.set<std::vector<MeshGeneratorName>>("inputs") = _inputs;
      params.set<std::vector<std::vector<unsigned int>>>("pattern") = _pattern;
      params.set<Real>("hexagon_size") = getReactorParam<Real>("assembly_pitch") / 2.0;
      params.set<MooseEnum>("hexagon_size_style") = "apothem";
      params.set<unsigned int>("background_intervals") = _background_intervals;
      params.set<bool>("create_outward_interface_boundaries") = false;
      // Initial block id used to define peripheral regions of assembly
      unsigned int assembly_block_id_start = 20000;

      const auto background_block_name = "RGMB_ASSEMBLY" + std::to_string(_assembly_type) + "_R0";
      const auto background_block_id = assembly_block_id_start;
      params.set<subdomain_id_type>("background_block_id") = background_block_id;
      params.set<SubdomainName>("background_block_name") = background_block_name;

      if (_duct_sizes.size() > 0)
      {
        std::vector<subdomain_id_type> duct_block_ids;
        std::vector<SubdomainName> duct_block_names;
        for (std::size_t duct_it = 0; duct_it < _duct_region_ids[0].size(); ++duct_it)
        {
          const auto duct_block_name =
              "RGMB_ASSEMBLY" + std::to_string(_assembly_type) + "_R" + std::to_string(duct_it + 1);
          const auto duct_block_id = assembly_block_id_start + duct_it + 1;
          duct_block_ids.push_back(duct_block_id);
          duct_block_names.push_back(duct_block_name);
        }

        params.set<std::vector<Real>>("duct_sizes") = _duct_sizes;
        params.set<std::vector<subdomain_id_type>>("duct_block_ids") = duct_block_ids;
        params.set<std::vector<SubdomainName>>("duct_block_names") = duct_block_names;
        params.set<std::vector<unsigned int>>("duct_intervals") = _duct_intervals;
      }

      params.set<boundary_id_type>("external_boundary_id") = _assembly_boundary_id;
      params.set<std::string>("external_boundary_name") = _assembly_boundary_name;

      addMeshSubgenerator("PatternedHexMeshGenerator", name() + "_pattern", params);

      // Pass mesh meta-data defined in subgenerator constructor to this MeshGenerator
      if (hasMeshProperty<Real>("pitch_meta", name() + "_pattern"))
        declareMeshProperty("pitch_meta", getMeshProperty<Real>("pitch_meta", name() + "_pattern"));
      if (hasMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta",
                                                     name() + "_pattern"))
        declareMeshProperty("num_sectors_per_side_meta",
                            getMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta",
                                                                       name() + "_pattern"));
      if (hasMeshProperty<bool>("is_control_drum_meta", name() + "_pattern"))
        declareMeshProperty("is_control_drum_meta",
                            getMeshProperty<bool>("is_control_drum_meta", name() + "_pattern"));
      if (hasMeshProperty<std::vector<Point>>("control_drum_positions", name() + "_pattern"))
        declareMeshProperty(
            "control_drum_positions",
            getMeshProperty<std::vector<Point>>("control_drum_positions", name() + "_pattern"));
      if (hasMeshProperty<std::vector<Real>>("control_drum_angles", name() + "_pattern"))
        declareMeshProperty(
            "control_drum_angles",
            getMeshProperty<std::vector<Real>>("control_drum_angles", name() + "_pattern"));
      if (hasMeshProperty<std::vector<std::vector<Real>>>("control_drums_azimuthal_meta",
                                                          name() + "_pattern"))
        declareMeshProperty("control_drums_azimuthal_meta",
                            getMeshProperty<std::vector<std::vector<Real>>>(
                                "control_drums_azimuthal_meta", name() + "_pattern"));
      if (hasMeshProperty<std::string>("position_file_name", name() + "_pattern"))
        declareMeshProperty(
            "position_file_name",
            getMeshProperty<std::string>("position_file_name", name() + "_pattern"));
      if (hasMeshProperty<Real>("pattern_pitch_meta", name() + "_pattern"))
        declareMeshProperty("pattern_pitch_meta",
                            getMeshProperty<Real>("pattern_pitch_meta", name() + "_pattern"));

      declareMeshProperty("background_region_ids", _background_region_id);
      declareMeshProperty("duct_region_ids", _duct_region_ids);
      declareMeshProperty("background_block_names", _background_block_name);
      declareMeshProperty("duct_block_names", _duct_block_names);
    }
  }

  std::string build_mesh_name = name() + "_delbds";

  // Remove outer pin sidesets created by PolygonConcentricCircleMeshGenerator
  {
    // Get outer boundaries of all constituent pins based on pin_type
    std::vector<BoundaryName> boundaries_to_delete = {};
    for (const auto & pattern_x : _pattern)
    {
      for (const auto & pattern_idx : pattern_x)
      {
        const auto pin_name = _inputs[pattern_idx];
        const auto pin_id = getMeshProperty<subdomain_id_type>("pin_type", pin_name);
        const BoundaryName boundary_name = "outer_pin_" + std::to_string(pin_id);
        if (!std::count(boundaries_to_delete.begin(), boundaries_to_delete.end(), boundary_name))
          boundaries_to_delete.push_back(boundary_name);
      }
    }
    auto params = _app.getFactory().getValidParams("BoundaryDeletionGenerator");

    params.set<MeshGeneratorName>("input") = name() + "_pattern";
    params.set<std::vector<BoundaryName>>("boundary_names") = boundaries_to_delete;

    addMeshSubgenerator("BoundaryDeletionGenerator", build_mesh_name, params);
  }

  for (auto pinMG : _inputs)
  {
    std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>> region_id_map =
        getMeshProperty<std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>>(
            "pin_region_ids", pinMG);
    _pin_region_id_map.insert(
        std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
            region_id_map.begin()->first, region_id_map.begin()->second));
    std::map<subdomain_id_type, std::vector<std::vector<std::string>>> block_name_map =
        getMeshProperty<std::map<subdomain_id_type, std::vector<std::vector<std::string>>>>(
            "pin_block_names", pinMG);
    _pin_block_name_map.insert(std::pair<subdomain_id_type, std::vector<std::vector<std::string>>>(
        block_name_map.begin()->first, block_name_map.begin()->second));
  }
  declareMeshProperty("pin_region_id_map", _pin_region_id_map);
  declareMeshProperty("pin_block_name_map", _pin_block_name_map);
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
      auto params = _app.getFactory().getValidParams("AdvancedExtruderGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_delbds";
      params.set<Point>("direction") = Point(0, 0, 1);
      params.set<std::vector<unsigned int>>("num_layers") =
          getMeshProperty<std::vector<unsigned int>>("axial_mesh_intervals", _reactor_params);
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
        plane_heights.push_back(z + plane_heights.back());

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
AssemblyMeshGenerator::generate()
{
  // Must be called to free the ReactorMeshParams mesh
  freeReactorMeshParams();

  // Update metadata at this point since values for these metadata only get set by PCCMG
  // at generate() stage
  if (hasMeshProperty<Real>("pattern_pitch_meta", name() + "_pattern"))
  {
    const auto pattern_pitch_meta =
        getMeshProperty<Real>("pattern_pitch_meta", name() + "_pattern");
    setMeshProperty("pattern_pitch_meta", pattern_pitch_meta);
  }

  // This generate() method will be called once the subgenerators that we depend on are
  // called. This is where we reassign subdomain ids/name in case they were merged when
  // stitching pins into an assembly. This is also where we set region_id and
  // assembly_type_id element integers.

  // Define all extra element names and integers
  std::string plane_id_name = "plane_id";
  std::string region_id_name = "region_id";
  std::string pin_type_id_name = "pin_type_id";
  std::string assembly_type_id_name = "assembly_type_id";
  std::string radial_id_name = "radial_id";
  const std::string default_block_name = "RGMB_ASSEMBLY" + std::to_string(_assembly_type);

  auto pin_type_id_int = getElemIntegerFromMesh(*(*_build_mesh), pin_type_id_name, true);
  auto region_id_int = getElemIntegerFromMesh(*(*_build_mesh), region_id_name, true);
  auto radial_id_int = getElemIntegerFromMesh(*(*_build_mesh), radial_id_name, true);

  auto assembly_type_id_int = getElemIntegerFromMesh(*(*_build_mesh), assembly_type_id_name);

  unsigned int plane_id_int = 0;
  if (_extrude)
    plane_id_int = getElemIntegerFromMesh(*(*_build_mesh), plane_id_name, true);

  // Get next free block ID in mesh in case subdomain ids need to be remapped
  auto next_block_id = MooseMeshUtils::getNextFreeSubdomainID(*(*(_build_mesh)));
  std::map<std::string, SubdomainID> rgmb_name_id_map;

  // Loop through all mesh elements and set region ids and reassign block IDs/names
  // if they were merged during pin stitching
  for (auto & elem : (*_build_mesh)->active_element_ptr_range())
  {
    elem->set_extra_integer(assembly_type_id_int, _assembly_type);
    const dof_id_type pin_type_id = elem->get_extra_integer(pin_type_id_int);
    const dof_id_type z_id = _extrude ? elem->get_extra_integer(plane_id_int) : 0;

    // Element is part of a pin mesh
    if (_pin_region_id_map.find(pin_type_id) != _pin_region_id_map.end())
    {
      // Get region ID from pin_type, z_id, and radial_idx
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
    else
    {
      // Assembly peripheral element (background / duct), set subdomains according
      // to user preferences and set pin type id to UINT16_MAX - 1 - peripheral index
      // Region id is inferred from z_id and peripheral_idx
      const auto base_block_id = elem->subdomain_id();
      const auto base_block_name = (*_build_mesh)->subdomain_name(base_block_id);

      // Check if block name has correct prefix
      std::string prefix = "RGMB_ASSEMBLY" + std::to_string(_assembly_type) + "_R";
      if (!(base_block_name.find(prefix, 0) == 0))
        continue;
      // Peripheral index is integer value of substring after prefix
      const unsigned int peripheral_idx = std::stoi(base_block_name.substr(prefix.length()));

      bool is_background_region = peripheral_idx == 0;

      subdomain_id_type pin_type = UINT16_MAX - 1 - peripheral_idx;
      elem->set_extra_integer(pin_type_id_int, pin_type);

      const auto elem_rid = (is_background_region ? _background_region_id[z_id]
                                                  : _duct_region_ids[z_id][peripheral_idx - 1]);
      elem->set_extra_integer(region_id_int, elem_rid);

      // Set element block name and block id
      auto elem_block_name = default_block_name;
      if (is_background_region && _has_background_block_name)
        elem_block_name += "_" + _background_block_name[z_id];
      if (!is_background_region && _has_duct_block_names)
        elem_block_name += "_" + _duct_block_names[z_id][peripheral_idx - 1];
      updateElementBlockNameId(
          *(*_build_mesh), elem, rgmb_name_id_map, elem_block_name, next_block_id);
    }
  }

  (*_build_mesh)->find_neighbors();

  return std::move(*_build_mesh);
}
