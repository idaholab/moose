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
                             "sized pins from PinMeshGenerator. "
                             "A hexagonal assembly will be placed inside of a bounding hexagon "
                             "consisting of a background region and, optionally,"
                             " duct regions.");
  // depletion id generation params are added
  addDepletionIDParams(params);

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
  MeshGeneratorName reactor_params =
      MeshGeneratorName(getMeshProperty<std::string>(RGMB::reactor_params_name, _inputs[0]));
  // Check that MG name for reactor params is consistent across all assemblies
  for (unsigned int i = 1; i < _inputs.size(); i++)
    if (getMeshProperty<std::string>(RGMB::reactor_params_name, _inputs[i]) != reactor_params)
      mooseError("The name of all reactor_params objects should be identical across all input pins "
                 "in the assembly.\n");

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

  Real base_pitch = 0.0;

  // Check constitutent pins do not have shared pin_type ids
  std::map<subdomain_id_type, std::string> pin_map_type_to_name;
  for (const auto i : index_range(_inputs))
  {
    auto pin = _inputs[i];
    if (i == 0)
      base_pitch = getMeshProperty<Real>(RGMB::pitch, pin);
    else
    {
      auto pitch = getMeshProperty<Real>(RGMB::pitch, pin);
      if (!MooseUtils::absoluteFuzzyEqual(pitch, base_pitch))
        mooseError("All pins within an assembly must have the same pitch");
    }
    if (getMeshProperty<bool>(RGMB::extruded, pin))
      mooseError("Pins that have already been extruded cannot be used in AssemblyMeshGenerator "
                 "definition.\n");
    const auto pin_type = getMeshProperty<subdomain_id_type>(RGMB::pin_type, pin);
    if (pin_map_type_to_name.find(pin_type) != pin_map_type_to_name.end() &&
        pin_map_type_to_name[pin_type] != pin)
      mooseError("Constituent pins have shared pin_type ids but different names. Each uniquely "
                 "defined pin in PinMeshGenerator must have its own pin_type id.");
    pin_map_type_to_name[pin_type] = pin;
  }
  auto assembly_pitch = getReactorParam<Real>(RGMB::assembly_pitch);

  unsigned int n_axial_levels =
      (_mesh_dimensions == 3)
          ? getReactorParam<std::vector<unsigned int>>(RGMB::axial_mesh_intervals).size()
          : 1;
  if (_geom_type == "Square")
  {
    const auto ny = _pattern.size();
    const auto nx = _pattern[0].size();
    if (_background_region_id.size() == 0)
    {
      if ((!MooseUtils::absoluteFuzzyEqual(base_pitch * ny, assembly_pitch)) ||
          (!MooseUtils::absoluteFuzzyEqual(base_pitch * nx, assembly_pitch)))
        mooseError(
            "Assembly pitch must be equal to lattice dimension times pin pitch for Cartesian "
            "assemblies with no background region");
      if (_background_intervals > 0)
        mooseError("\"background_region_id\" must be defined if \"background_intervals\" is "
                   "greater than 0");
    }
    else
    {
      if ((base_pitch * ny > assembly_pitch) || (base_pitch * nx > assembly_pitch))
        mooseError(
            "Assembly pitch must be larger than lattice dimension times pin pitch for Cartesian "
            "assemblies with background region");
      if (_background_intervals == 0)
        mooseError("\"background_intervals\" must be greater than 0 if \"background_region_id\" is "
                   "defined");
      if (_background_region_id.size() != n_axial_levels)
        mooseError(
            "The size of background_region_id must be equal to the number of axial levels as "
            "defined in the ReactorMeshParams object");
    }
  }
  else
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
    if (getReactorParam<bool>(RGMB::region_id_as_block_name))
      paramError("background_block_name",
                 "If ReactorMeshParams/region_id_as_block_name is set, background_block_name "
                 "should not be specified in AssemblyMeshGenerator");
    _has_background_block_name = true;
    _background_block_name = getParam<std::vector<std::string>>("background_block_name");
    if (_background_region_id.size() != _background_block_name.size())
      mooseError("The size of background_block_name must match the size of background_region_id");
  }
  else
    _has_background_block_name = false;

  if (isParamValid("duct_block_names"))
  {
    if (getReactorParam<bool>(RGMB::region_id_as_block_name))
      paramError("duct_block_names",
                 "If ReactorMeshParams/region_id_as_block_name is set, duct_block_names should not "
                 "be specified in AssemblyMeshGenerator");
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

  // No subgenerators will be called if option to bypass mesh generators is enabled
  if (!getReactorParam<bool>(RGMB::bypass_meshgen))
  {
    // Declare dependency of inputs to sub generator calls. If mesh generation
    declareMeshesForSub("inputs");

    _assembly_boundary_id = RGMB::ASSEMBLY_BOUNDARY_ID_START + _assembly_type;
    _assembly_boundary_name = RGMB::ASSEMBLY_BOUNDARY_NAME_PREFIX + std::to_string(_assembly_type);

    // Call PatternedHexMeshGenerator or PatternedCartesianMeshGenerator to stitch assembly
    {
      const auto patterned_mg_name =
          _geom_type == "Hex" ? "PatternedHexMeshGenerator" : "PatternedCartesianMeshGenerator";
      auto params = _app.getFactory().getValidParams(patterned_mg_name);

      if (_geom_type == "Hex")
      {
        params.set<Real>("hexagon_size") = getReactorParam<Real>(RGMB::assembly_pitch) / 2.0;
        params.set<MooseEnum>("hexagon_size_style") = "apothem";
      }
      else
      {
        if (_background_region_id.size() == 0)
          params.set<MooseEnum>("pattern_boundary") = "none";
        else
        {
          params.set<MooseEnum>("pattern_boundary") = "expanded";
          params.set<Real>("square_size") = getReactorParam<Real>(RGMB::assembly_pitch);
          params.set<bool>("uniform_mesh_on_sides") = true;
        }
      }

      params.set<std::vector<std::string>>("id_name") = {"pin_id"};
      params.set<std::vector<MooseEnum>>("assign_type") = {
          MooseEnum("cell", "cell")}; // give elems IDs relative to position in assembly
      params.set<std::vector<MeshGeneratorName>>("inputs") = _inputs;
      params.set<std::vector<std::vector<unsigned int>>>("pattern") = _pattern;
      params.set<bool>("create_outward_interface_boundaries") = false;

      if (_background_intervals > 0)
      {
        params.set<unsigned int>("background_intervals") = _background_intervals;
        // Initial block id used to define peripheral regions of assembly

        const auto background_block_name =
            RGMB::ASSEMBLY_BLOCK_NAME_PREFIX + std::to_string(_assembly_type) + "_R0";
        const auto background_block_id = RGMB::ASSEMBLY_BLOCK_ID_START;
        params.set<subdomain_id_type>("background_block_id") = background_block_id;
        params.set<SubdomainName>("background_block_name") = background_block_name;
      }

      if (_duct_sizes.size() > 0)
      {
        std::vector<subdomain_id_type> duct_block_ids;
        std::vector<SubdomainName> duct_block_names;
        for (const auto duct_it : index_range(_duct_region_ids[0]))
        {
          const auto duct_block_name = RGMB::ASSEMBLY_BLOCK_NAME_PREFIX +
                                       std::to_string(_assembly_type) + "_R" +
                                       std::to_string(duct_it + 1);
          const auto duct_block_id = RGMB::ASSEMBLY_BLOCK_ID_START + duct_it + 1;
          duct_block_ids.push_back(duct_block_id);
          duct_block_names.push_back(duct_block_name);
        }

        params.set<std::vector<Real>>("duct_sizes") = _duct_sizes;
        params.set<std::vector<subdomain_id_type>>("duct_block_ids") = duct_block_ids;
        params.set<std::vector<SubdomainName>>("duct_block_names") = duct_block_names;
        params.set<std::vector<unsigned int>>("duct_intervals") = _duct_intervals;
      }

      params.set<boundary_id_type>("external_boundary_id") = _assembly_boundary_id;
      params.set<BoundaryName>("external_boundary_name") = _assembly_boundary_name;

      addMeshSubgenerator(patterned_mg_name, name() + "_pattern", params);

      // Pass mesh meta-data defined in subgenerator constructor to this MeshGenerator
      copyMeshProperty<bool>("is_control_drum_meta", name() + "_pattern");
      copyMeshProperty<std::vector<Point>>("control_drum_positions", name() + "_pattern");
      copyMeshProperty<std::vector<Real>>("control_drum_angles", name() + "_pattern");
      copyMeshProperty<std::vector<std::vector<Real>>>("control_drums_azimuthal_meta",
                                                       name() + "_pattern");
      copyMeshProperty<std::string>("position_file_name", name() + "_pattern");
      copyMeshProperty<Real>("pattern_pitch_meta", name() + "_pattern");
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
          const auto pin_id = getMeshProperty<subdomain_id_type>(RGMB::pin_type, pin_name);
          const BoundaryName boundary_name =
              RGMB::PIN_BOUNDARY_NAME_PREFIX + std::to_string(pin_id);
          if (!std::count(boundaries_to_delete.begin(), boundaries_to_delete.end(), boundary_name))
            boundaries_to_delete.push_back(boundary_name);
        }
      }
      auto params = _app.getFactory().getValidParams("BoundaryDeletionGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_pattern";
      params.set<std::vector<BoundaryName>>("boundary_names") = boundaries_to_delete;

      addMeshSubgenerator("BoundaryDeletionGenerator", build_mesh_name, params);
    }

    // Modify outermost mesh interval to enable flexible assembly stitching
    const auto use_flexible_stitching = getReactorParam<bool>(RGMB::flexible_assembly_stitching);
    if (use_flexible_stitching)
    {
      generateFlexibleAssemblyBoundaries();
      build_mesh_name = name() + "_fpg_delbds";
    }

    for (auto pinMG : _inputs)
    {
      std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>> region_id_map =
          getMeshProperty<std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>>(
              RGMB::pin_region_ids, pinMG);
      _pin_region_id_map.insert(
          std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
              region_id_map.begin()->first, region_id_map.begin()->second));
      subdomain_id_type pin_type_id = getMeshProperty<subdomain_id_type>(RGMB::pin_type, pinMG);
      std::vector<std::vector<std::string>> pin_block_names =
          getMeshProperty<std::vector<std::vector<std::string>>>(RGMB::pin_block_names, pinMG);
      _pin_block_name_map.insert(
          std::pair<subdomain_id_type, std::vector<std::vector<std::string>>>(pin_type_id,
                                                                              pin_block_names));
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
AssemblyMeshGenerator::generateMetadata()
{
  // Declare metadata for use in downstream mesh generators
  declareMeshProperty(RGMB::assembly_type, _assembly_type);
  declareMeshProperty(RGMB::pitch, getReactorParam<Real>(RGMB::assembly_pitch));
  declareMeshProperty(RGMB::background_region_id, _background_region_id);
  declareMeshProperty(RGMB::background_block_name, _background_block_name);
  declareMeshProperty(RGMB::duct_halfpitches, _duct_sizes);
  declareMeshProperty(RGMB::duct_region_ids, _duct_region_ids);
  declareMeshProperty(RGMB::duct_block_names, _duct_block_names);
  declareMeshProperty(RGMB::is_homogenized, false);
  declareMeshProperty(RGMB::is_single_pin, false);
  declareMeshProperty(RGMB::extruded, _extrude && _mesh_dimensions == 3);
  declareMeshProperty(RGMB::is_control_drum, false);
  // Following metadata is only relevant if an output mesh is generated by RGMB
  if (!getReactorParam<bool>(RGMB::bypass_meshgen))
  {
    declareMeshProperty(RGMB::pin_region_id_map, _pin_region_id_map);
    declareMeshProperty(RGMB::pin_block_name_map, _pin_block_name_map);
  }

  // Determine constituent pin names and define lattice as metadata
  std::vector<std::vector<int>> pin_name_lattice;
  std::vector<std::string> input_pin_names;
  for (const auto i : index_range(_pattern))
  {
    std::vector<int> pin_name_idx(_pattern[i].size());
    for (const auto j : index_range(_pattern[i]))
    {
      const auto input_pin_name = _inputs[_pattern[i][j]];
      const auto it = std::find(input_pin_names.begin(), input_pin_names.end(), input_pin_name);
      if (it == input_pin_names.end())
      {
        pin_name_idx[j] = input_pin_names.size();
        input_pin_names.push_back(input_pin_name);
      }
      else
        pin_name_idx[j] = it - input_pin_names.begin();
    }
    pin_name_lattice.push_back(pin_name_idx);
  }
  declareMeshProperty(RGMB::pin_names, input_pin_names);
  declareMeshProperty(RGMB::pin_lattice, pin_name_lattice);
}

void
AssemblyMeshGenerator::generateFlexibleAssemblyBoundaries()
{
  // Assemblies that invoke this method have constituent pin lattice, delete outermost background or
  // duct region (if present)
  SubdomainName block_to_delete = "";
  if (_background_region_id.size() == 0)
    mooseError("Attempting to use flexible stitching on assembly " + name() +
               " that does not have a background region. This is not yet supported.");
  const auto radial_index = _duct_region_ids.size() == 0 ? 0 : _duct_region_ids[0].size();
  block_to_delete = RGMB::ASSEMBLY_BLOCK_NAME_PREFIX + std::to_string(_assembly_type) + "_R" +
                    std::to_string(radial_index);

  {
    // Invoke BlockDeletionGenerator to delete outermost mesh interval of assembly
    auto params = _app.getFactory().getValidParams("BlockDeletionGenerator");

    params.set<std::vector<SubdomainName>>("block") = {block_to_delete};
    params.set<MeshGeneratorName>("input") = name() + "_delbds";

    addMeshSubgenerator("BlockDeletionGenerator", name() + "_del_outer", params);
  }
  {
    // Invoke FlexiblePatternGenerator to triangulate deleted mesh region
    auto params = _app.getFactory().getValidParams("FlexiblePatternGenerator");

    params.set<std::vector<MeshGeneratorName>>("inputs") = {name() + "_del_outer"};
    params.set<std::vector<libMesh::Point>>("extra_positions") = {libMesh::Point(0, 0, 0)};
    params.set<std::vector<unsigned int>>("extra_positions_mg_indices") = {0};
    params.set<bool>("use_auto_area_func") = true;
    params.set<MooseEnum>("boundary_type") = (_geom_type == "Hex") ? "HEXAGON" : "CARTESIAN";
    params.set<unsigned int>("boundary_sectors") =
        getReactorParam<unsigned int>(RGMB::num_sectors_flexible_stitching);
    params.set<Real>("boundary_size") = getReactorParam<Real>(RGMB::assembly_pitch);
    params.set<boundary_id_type>("external_boundary_id") = _assembly_boundary_id;
    params.set<BoundaryName>("external_boundary_name") = _assembly_boundary_name;
    params.set<SubdomainName>("background_subdomain_name") =
        block_to_delete + RGMB::TRI_BLOCK_NAME_SUFFIX;
    params.set<bool>("verify_holes") = false;
    params.set<unsigned short>("background_subdomain_id") = RGMB::ASSEMBLY_BLOCK_ID_TRI_FLEXIBLE;

    addMeshSubgenerator("FlexiblePatternGenerator", name() + "_fpg", params);
  }
  {
    // Delete extra boundary created by FlexiblePatternGenerator
    auto params = _app.getFactory().getValidParams("BoundaryDeletionGenerator");

    params.set<MeshGeneratorName>("input") = name() + "_fpg";
    params.set<std::vector<BoundaryName>>("boundary_names") = {std::to_string(1)};

    addMeshSubgenerator("BoundaryDeletionGenerator", name() + "_fpg_delbds", params);
  }
}

std::unique_ptr<MeshBase>
AssemblyMeshGenerator::generate()
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
  const std::string default_block_name =
      RGMB::ASSEMBLY_BLOCK_NAME_PREFIX + std::to_string(_assembly_type);

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
      else if (getReactorParam<bool>(RGMB::region_id_as_block_name))
        elem_block_name += "_REG" + std::to_string(elem_rid);
      if (elem->type() == TRI3 || elem->type() == PRISM6)
        elem_block_name += RGMB::TRI_BLOCK_NAME_SUFFIX;
      updateElementBlockNameId(
          *(*_build_mesh), elem, rgmb_name_id_map, elem_block_name, next_block_id);
    }
    else
    {
      // Assembly peripheral element (background / duct), set subdomains according
      // to user preferences and set pin type id to RGMB::MAX_PIN_TYPE_ID - peripheral index
      // Region id is inferred from z_id and peripheral_idx
      const auto base_block_id = elem->subdomain_id();
      const auto base_block_name = (*_build_mesh)->subdomain_name(base_block_id);

      // Check if block name has correct prefix
      std::string prefix = RGMB::ASSEMBLY_BLOCK_NAME_PREFIX + std::to_string(_assembly_type) + "_R";
      if (!(base_block_name.find(prefix, 0) == 0))
        continue;
      // Peripheral index is integer value of substring after prefix
      const unsigned int peripheral_idx = std::stoi(base_block_name.substr(prefix.length()));

      bool is_background_region = peripheral_idx == 0;

      subdomain_id_type pin_type = RGMB::MAX_PIN_TYPE_ID - peripheral_idx;
      elem->set_extra_integer(pin_type_id_int, pin_type);

      const auto elem_rid = (is_background_region ? _background_region_id[z_id]
                                                  : _duct_region_ids[z_id][peripheral_idx - 1]);
      elem->set_extra_integer(region_id_int, elem_rid);

      // Set element block name and block id
      auto elem_block_name = default_block_name;
      if (getReactorParam<bool>(RGMB::region_id_as_block_name))
        elem_block_name += "_REG" + std::to_string(elem_rid);
      else if (is_background_region && _has_background_block_name)
        elem_block_name += "_" + _background_block_name[z_id];
      else if (!is_background_region && _has_duct_block_names)
        elem_block_name += "_" + _duct_block_names[z_id][peripheral_idx - 1];
      updateElementBlockNameId(
          *(*_build_mesh), elem, rgmb_name_id_map, elem_block_name, next_block_id);
    }
  }

  if (getParam<bool>("generate_depletion_id"))
  {
    const MooseEnum option = getParam<MooseEnum>("depletion_id_type");
    addDepletionId(*(*_build_mesh), option, DepletionIDGenerationLevel::Assembly, _extrude);
  }

  // Mark mesh as not prepared, as block IDs were re-assigned in this method
  (*_build_mesh)->set_isnt_prepared();

  return std::move(*_build_mesh);
}
