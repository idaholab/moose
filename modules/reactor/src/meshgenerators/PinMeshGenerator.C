//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PinMeshGenerator.h"

#include "ReactorGeometryMeshBuilderBase.h"
#include <cmath>
#include "MooseApp.h"
#include "MooseMeshUtils.h"
#include "Factory.h"
#include "libmesh/elem.h"

registerMooseObject("ReactorApp", PinMeshGenerator);

InputParameters
PinMeshGenerator::validParams()
{
  auto params = ReactorGeometryMeshBuilderBase::validParams();

  params.addRequiredParam<MeshGeneratorName>(
      "reactor_params",
      "The ReactorMeshParams MeshGenerator that is the basis for this component conformal mesh.");

  params.addRequiredParam<subdomain_id_type>("pin_type",
                                             "The integer ID for this pin type definition");

  params.addRequiredRangeCheckedParam<Real>(
      "pitch", "pitch>0.0", "The pitch for the outermost boundary polygon");

  params.addRangeCheckedParam<unsigned int>(
      "num_sectors", "num_sectors>0", "Number of azimuthal sectors in each quadrant");

  params.addRangeCheckedParam<std::vector<Real>>(
      "ring_radii",
      "ring_radii>0.0",
      "Radii of major concentric circles of the pin. If unspecified, no pin is present.");

  params.addRangeCheckedParam<std::vector<Real>>(
      "duct_halfpitch",
      "duct_halfpitch>0.0",
      "Apothem of the ducts. If unspecified, no duct is present.");

  params.addRangeCheckedParam<std::vector<unsigned int>>(
      "mesh_intervals",
      std::vector<unsigned int>{1},
      "mesh_intervals>0",
      "The number of meshing intervals for each region starting at the center. Parameter should be "
      "size:"
      "((length(ring_radii) + length(duct_halfpitch) + 1");

  params.addParam<std::vector<std::vector<std::string>>>(
      "block_names",
      "Block names for each radial and axial zone. "
      "Inner indexing is radial zones (pin/background/duct), outer indexing is axial");

  params.addParam<std::vector<std::vector<subdomain_id_type>>>(
      "region_ids",
      "IDs for each radial and axial zone for assignment of region_id extra element "
      "id. "
      "Inner indexing is radial zones (pin/background/duct), outer indexing is axial");

  params.addParam<bool>("extrude",
                        false,
                        "Determines if this is the final step in the geometry construction"
                        " and extrudes the 2D geometry to 3D. If this is true then this mesh "
                        "cannot be used in further mesh building in the Reactor workflow");
  params.addParam<bool>(
      "homogenized", false, "Determines whether homogenized pin mesh should be generated");
  params.addParam<bool>(
      "use_as_assembly", false, "Determines whether pin mesh should be used as an assembly mesh");

  params.addParam<bool>(
      "quad_center_elements", true, "Whether the center elements are quad or triangular.");
  params.addParamNamesToGroup("region_ids pin_type", "ID assigment");
  params.addParamNamesToGroup(
      "mesh_intervals ring_radii num_sectors pin_type homogenized use_as_assembly",
      "Pin specifications");
  params.addParamNamesToGroup("mesh_intervals duct_halfpitch num_sectors", "Duct specifications");

  params.addClassDescription("This PinMeshGenerator object is designed to generate pin-like "
                             "structures, with IDs, from a reactor geometry. "
                             "Whether it be a square or hexagonal pin, they are divided into three "
                             "substructures - the innermost "
                             "radial pin regions, the single bridging background region, and the "
                             "square or hexagonal ducts regions.");

  return params;
}

PinMeshGenerator::PinMeshGenerator(const InputParameters & parameters)
  : ReactorGeometryMeshBuilderBase(parameters),
    _pin_type(getParam<subdomain_id_type>("pin_type")),
    _pitch(getParam<Real>("pitch")),
    _num_sectors(isParamValid("num_sectors") ? getParam<unsigned int>("num_sectors") : 0),
    _ring_radii(isParamValid("ring_radii") ? getParam<std::vector<Real>>("ring_radii")
                                           : std::vector<Real>()),
    _duct_halfpitch(isParamValid("duct_halfpitch") ? getParam<std::vector<Real>>("duct_halfpitch")
                                                   : std::vector<Real>()),
    _intervals(getParam<std::vector<unsigned int>>("mesh_intervals")),
    _region_ids(isParamValid("region_ids")
                    ? getParam<std::vector<std::vector<subdomain_id_type>>>("region_ids")
                    : std::vector<std::vector<subdomain_id_type>>()),
    _extrude(getParam<bool>("extrude")),
    _quad_center(getParam<bool>("quad_center_elements")),
    _homogenized(getParam<bool>("homogenized")),
    _is_assembly(getParam<bool>("use_as_assembly"))
{
  // Initialize ReactorMeshParams object
  initializeReactorMeshParams(getParam<MeshGeneratorName>("reactor_params"));

  _mesh_dimensions = getReactorParam<unsigned int>(RGMB::mesh_dimensions);
  _mesh_geometry = getReactorParam<std::string>(RGMB::mesh_geometry);

  if (_is_assembly)
  {
    auto assembly_pitch = getReactorParam<Real>(RGMB::assembly_pitch);
    if (assembly_pitch != _pitch)
      mooseError("Pitch defined in PinMeshGenerator must match assembly_pitch defined in "
                 "ReactorMeshParams if use_as_assembly is set to true");
  }

  if (_extrude && _mesh_dimensions != 3)
    paramError("extrude",
               "In order to extrude this mesh, ReactorMeshParams/dim needs to be set to 3\n");
  if (_extrude && (!hasReactorParam<boundary_id_type>(RGMB::top_boundary_id) ||
                   !hasReactorParam<boundary_id_type>(RGMB::bottom_boundary_id)))
    mooseError("Both top_boundary_id and bottom_boundary_id must be provided in ReactorMeshParams "
               "if using extruded geometry");

  if (_homogenized)
  {
    if (_mesh_geometry == "Square")
      mooseError("Homogenization in PinMeshGenerator is only supported for hexagonal geometries");
    const std::vector<std::string> disallowed_parameters = {
        "num_sectors", "ring_radii", "duct_halfpitch", "mesh_intervals"};
    for (const auto & parameter : disallowed_parameters)
      if (parameters.isParamSetByUser(parameter))
        paramError(parameter,
                   "Parameter " + parameter + " should not be defined for a homogenized pin mesh");
  }
  else
  {
    if (_num_sectors == 0)
      mooseError(
          "Number of sectors must be assigned with parameter num_sectors for non-homogenized pins");
    if (_intervals.size() != (_ring_radii.size() + _duct_halfpitch.size() + 1))
      mooseError(
          "The number of mesh intervals must be equal to the number of annular regions + the "
          "number of duct regions + 1"
          " for the region between the rings and ducts\n");
  }

  if (isParamValid("region_ids"))
  {
    unsigned int n_axial_levels =
        (_mesh_dimensions == 3)
            ? getReactorParam<std::vector<unsigned int>>(RGMB::axial_mesh_intervals).size()
            : 1;
    if (_region_ids.size() != n_axial_levels)
      mooseError("The size of region IDs must be equal to the number of axial levels as defined in "
                 "the ReactorMeshParams object");
    if (_region_ids[0].size() != (_ring_radii.size() + _duct_halfpitch.size() + 1))
      mooseError("The number of region IDs given needs to be one more than the number of "
                 "ring_radii + the number of duct_radii\n");
  }
  else
  {
    mooseError("Region IDs must be assigned with parameter region_ids");
  }
  if (isParamValid("block_names"))
  {
    if (getReactorParam<bool>(RGMB::region_id_as_block_name))
      paramError("block_names",
                 "If ReactorMeshParams/region_id_as_block_name is set, block_names should not be "
                 "specified in PinMeshGenerator");
    _has_block_names = true;
    _block_names = getParam<std::vector<std::vector<std::string>>>("block_names");
    if (_region_ids.size() != _block_names.size())
      mooseError("The size of block_names must match the size of region_ids");
    for (const auto i : index_range(_region_ids))
      if (_region_ids[i].size() != _block_names[i].size())
        mooseError("The size of block_names must match the size of region_ids");
  }
  else
    _has_block_names = false;

  const auto use_flexible_stitching = getReactorParam<bool>(RGMB::flexible_assembly_stitching);
  std::string build_mesh_name;

  // No subgenerators will be called if option to bypass mesh generators is enabled
  if (!getReactorParam<bool>(RGMB::bypass_meshgen))
  {
    if (_homogenized)
    {
      // If flexible assembly stitching is invoked and this is a homogeneous assembly mesh, do not
      // call mesh subgenerators here. The homogeneous assembly mesh should be created entirely in
      // generateFlexibleAssemblyBoundaries()
      bool skip_assembly_generation = _is_assembly && use_flexible_stitching;

      auto params = _app.getFactory().getValidParams("SimpleHexagonGenerator");

      params.set<Real>("hexagon_size") = _pitch / 2.0;
      params.set<boundary_id_type>("external_boundary_id") =
          RGMB::PIN_BOUNDARY_ID_START + _pin_type;
      const auto boundary_name =
          (_is_assembly ? RGMB::ASSEMBLY_BOUNDARY_NAME_PREFIX : RGMB::PIN_BOUNDARY_NAME_PREFIX) +
          std::to_string(_pin_type);
      params.set<BoundaryName>("external_boundary_name") = boundary_name;
      params.set<std::vector<subdomain_id_type>>("block_id") = {
          _quad_center ? RGMB::PIN_BLOCK_ID_START : RGMB::PIN_BLOCK_ID_TRI};
      params.set<MooseEnum>("element_type") = _quad_center ? "QUAD" : "TRI";
      auto block_name = RGMB::PIN_BLOCK_NAME_PREFIX + std::to_string(_pin_type) + "_R0";
      if (!_quad_center)
        block_name += RGMB::TRI_BLOCK_NAME_SUFFIX;
      params.set<std::vector<SubdomainName>>("block_name") = {block_name};

      if (!skip_assembly_generation)
      {
        build_mesh_name = name() + "_2D";
        addMeshSubgenerator("SimpleHexagonGenerator", build_mesh_name, params);
      }
    }
    else
    {
      // Define all id variables used in the pin
      std::vector<unsigned int> ring_intervals;
      std::vector<subdomain_id_type> ring_blk_ids;
      std::vector<SubdomainName> ring_blk_names;
      unsigned int background_intervals = 1;
      std::vector<subdomain_id_type> background_blk_ids;
      std::vector<SubdomainName> background_blk_names;
      std::vector<unsigned int> duct_intervals;
      std::vector<subdomain_id_type> duct_blk_ids;
      std::vector<SubdomainName> duct_blk_names;

      for (const auto i : index_range(_intervals))
      {
        const auto block_name =
            RGMB::PIN_BLOCK_NAME_PREFIX + std::to_string(_pin_type) + "_R" + std::to_string(i);
        const auto block_id = RGMB::PIN_BLOCK_ID_START + i;

        if (i < _ring_radii.size())
        {
          ring_intervals.push_back(_intervals[i]);
          ring_blk_ids.push_back(block_id);
          ring_blk_names.push_back(block_name);
        }
        else if (i > _ring_radii.size())
        {
          duct_intervals.push_back(_intervals[i]);
          duct_blk_ids.push_back(block_id);
          duct_blk_names.push_back(block_name);
        }
        else
        {
          background_intervals = _intervals[i];
          background_blk_ids.push_back(block_id);
          background_blk_names.push_back(block_name);
        }
      }
      if (ring_intervals.size() > 0)
      {
        if (ring_intervals.front() != 1)
        {
          // If quad center elements, copy element at beginning of block names and
          // block ids. Otherwise add RGMB::TRI_BLOCK_NAME_SUFFIX to block names and generate new
          // block id
          if (_quad_center)
          {
            ring_blk_ids.insert(ring_blk_ids.begin(), ring_blk_ids.front());
            ring_blk_names.insert(ring_blk_names.begin(), ring_blk_names.front());
          }
          else
          {
            const auto block_name = ring_blk_names.front() + RGMB::TRI_BLOCK_NAME_SUFFIX;
            const auto block_id = RGMB::PIN_BLOCK_ID_TRI;
            ring_blk_ids.insert(ring_blk_ids.begin(), block_id);
            ring_blk_names.insert(ring_blk_names.begin(), block_name);
          }
        }
        // Add RGMB::TRI_BLOCK_NAME_SUFFIX if only one radial region and tri center elements
        else if (!_quad_center)
        {
          ring_blk_ids[0] = RGMB::PIN_BLOCK_ID_TRI;
          ring_blk_names[0] += RGMB::TRI_BLOCK_NAME_SUFFIX;
        }
      }
      else
      {
        if (background_intervals > 1)
        {
          // If quad center elements, copy element at beginning of block names and
          // block ids. Otherwise add RGMB::TRI_BLOCK_NAME_SUFFIX to block names and generate new
          // block id
          if (_quad_center)
          {
            background_blk_ids.insert(background_blk_ids.begin(), background_blk_ids.front());
            background_blk_names.insert(background_blk_names.begin(), background_blk_names.front());
          }
          else
          {
            const auto block_name = background_blk_names.front() + RGMB::TRI_BLOCK_NAME_SUFFIX;
            const auto block_id = RGMB::PIN_BLOCK_ID_TRI;
            background_blk_ids.insert(background_blk_ids.begin(), block_id);
            background_blk_names.insert(background_blk_names.begin(), block_name);
          }
        }
        // Add RGMB::TRI_BLOCK_NAME_SUFFIX if only one background region and tri center elements
        // and no ring regions
        else if (!_quad_center)
        {
          background_blk_ids[0] = RGMB::PIN_BLOCK_ID_TRI;
          background_blk_names[0] += RGMB::TRI_BLOCK_NAME_SUFFIX;
        }
      }

      // If flexible assembly stitching is invoked and this is an assembly mesh with only a
      // background region, do not call mesh subgenerators here. This assembly mesh should be
      // created entirely in generateFlexibleAssemblyBoundaries()
      bool skip_assembly_generation =
          _is_assembly && use_flexible_stitching && _intervals.size() == 1;

      if (!skip_assembly_generation)
      {
        // Generate Cartesian/hex pin using PolygonConcentricCircleMeshGenerator
        {
          // Get and assign parameters for the main geometry feature of the Pin
          // which is created with a PolygonConcentricCircleMeshGenerator subgenerator
          auto params = _app.getFactory().getValidParams("PolygonConcentricCircleMeshGenerator");
          params.set<bool>("preserve_volumes") = true;
          params.set<bool>("quad_center_elements") = _quad_center;
          params.set<MooseEnum>("polygon_size_style") = "apothem";
          params.set<Real>("polygon_size") = _pitch / 2.0;
          params.set<boundary_id_type>("external_boundary_id") =
              RGMB::PIN_BOUNDARY_ID_START + _pin_type;
          const auto boundary_name = (_is_assembly ? RGMB::ASSEMBLY_BOUNDARY_NAME_PREFIX
                                                   : RGMB::PIN_BOUNDARY_NAME_PREFIX) +
                                     std::to_string(_pin_type);
          params.set<BoundaryName>("external_boundary_name") = boundary_name;
          bool flat_side_up = (_mesh_geometry == "Square");
          params.set<bool>("flat_side_up") = flat_side_up;
          params.set<bool>("create_outward_interface_boundaries") = false;

          const auto num_sides = (_mesh_geometry == "Square") ? 4 : 6;
          params.set<unsigned int>("num_sides") = num_sides;
          params.set<std::vector<unsigned int>>("num_sectors_per_side") =
              std::vector<unsigned int>(num_sides, _num_sectors);

          if (ring_intervals.size() > 0)
          {
            params.set<std::vector<Real>>("ring_radii") = _ring_radii;
            params.set<std::vector<subdomain_id_type>>("ring_block_ids") = ring_blk_ids;
            params.set<std::vector<SubdomainName>>("ring_block_names") = ring_blk_names;
            params.set<std::vector<unsigned int>>("ring_intervals") = ring_intervals;
          }

          params.set<std::vector<subdomain_id_type>>("background_block_ids") = background_blk_ids;
          params.set<std::vector<SubdomainName>>("background_block_names") = background_blk_names;
          params.set<unsigned int>("background_intervals") = background_intervals;

          if (duct_intervals.size() > 0)
          {
            params.set<MooseEnum>("duct_sizes_style") = "apothem";
            params.set<std::vector<Real>>("duct_sizes") = _duct_halfpitch;
            params.set<std::vector<subdomain_id_type>>("duct_block_ids") = duct_blk_ids;
            params.set<std::vector<SubdomainName>>("duct_block_names") = duct_blk_names;
            params.set<std::vector<unsigned int>>("duct_intervals") = duct_intervals;
          }

          addMeshSubgenerator("PolygonConcentricCircleMeshGenerator", name() + "_2D", params);
        }

        // Remove extra sidesets created by PolygonConcentricCircleMeshGenerator
        {
          auto params = _app.getFactory().getValidParams("BoundaryDeletionGenerator");

          params.set<MeshGeneratorName>("input") = name() + "_2D";

          auto num_sides = (_mesh_geometry == "Square") ? 4 : 6;
          std::vector<BoundaryName> boundaries_to_delete = {};
          for (const auto i : make_range(num_sides))
            boundaries_to_delete.insert(boundaries_to_delete.end(),
                                        {std::to_string(10001 + i), std::to_string(15001 + i)});
          params.set<std::vector<BoundaryName>>("boundary_names") = boundaries_to_delete;

          build_mesh_name = name() + "_delbds";
          addMeshSubgenerator("BoundaryDeletionGenerator", build_mesh_name, params);
        }
      }
    }

    // For pin acting as assembly, modify outermost mesh interval to enable flexible assembly
    // stitching
    if (_is_assembly && use_flexible_stitching)
    {
      generateFlexibleAssemblyBoundaries();
      build_mesh_name = name() + "_fpg_delbds";
    }

    // Pass mesh meta-data defined in subgenerator constructor to this MeshGenerator
    if (hasMeshProperty<Real>("pitch_meta", name() + "_2D"))
      copyMeshProperty<Real>("pitch_meta", name() + "_2D");
    if (hasMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta", name() + "_2D"))
      copyMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta", name() + "_2D");
    if (hasMeshProperty<Real>("max_radius_meta", name() + "_2D"))
      copyMeshProperty<Real>("max_radius_meta", name() + "_2D");
    if (hasMeshProperty<unsigned int>("background_intervals_meta", name() + "_2D"))
      copyMeshProperty<unsigned int>("background_intervals_meta", name() + "_2D");
    if (hasMeshProperty<dof_id_type>("node_id_background_meta", name() + "_2D"))
      copyMeshProperty<dof_id_type>("node_id_background_meta", name() + "_2D");

    if (_is_assembly)
      declareMeshProperty("pattern_pitch_meta", getReactorParam<Real>(RGMB::assembly_pitch));
    else if (hasMeshProperty<Real>("pattern_pitch_meta", name() + "_2D"))
      copyMeshProperty<Real>("pattern_pitch_meta", name() + "_2D");
    declareMeshProperty("is_control_drum_meta", false);

    if (_extrude && _mesh_dimensions == 3)
      build_mesh_name = callExtrusionMeshSubgenerators(build_mesh_name);

    // Store final mesh subgenerator
    _build_mesh = &getMeshByName(build_mesh_name);
  }

  generateMetadata();
}

void
PinMeshGenerator::generateFlexibleAssemblyBoundaries()
{
  SubdomainName outermost_block_name;
  bool has_single_mesh_interval;

  // Assemblies that invoke this method are either homogenized or have a single pin. First check if
  // the assembly only has a single region. Otherwise, determine the outermost region for deletion
  if (_homogenized || (_intervals.size() == 1))
  {
    outermost_block_name = RGMB::PIN_BLOCK_NAME_PREFIX + std::to_string(_pin_type) + "_R0";
    has_single_mesh_interval = true;
  }
  else
  {
    outermost_block_name = RGMB::PIN_BLOCK_NAME_PREFIX + std::to_string(_pin_type) + "_R" +
                           std::to_string(_intervals.size() - 1);
    has_single_mesh_interval = false;

    // Invoke BlockDeletionGenerator to delete outermost mesh interval of assembly
    auto params = _app.getFactory().getValidParams("BlockDeletionGenerator");

    params.set<std::vector<SubdomainName>>("block") = {outermost_block_name};
    params.set<MeshGeneratorName>("input") = _homogenized ? name() + "_2D" : name() + "_delbds";

    addMeshSubgenerator("BlockDeletionGenerator", name() + "_del_outer", params);
  }

  {
    // Invoke FlexiblePatternGenerator to triangulate deleted mesh interval
    auto params = _app.getFactory().getValidParams("FlexiblePatternGenerator");

    if (has_single_mesh_interval)
      params.set<std::vector<MeshGeneratorName>>("inputs") = {};
    else
    {
      params.set<std::vector<MeshGeneratorName>>("inputs") = {name() + "_del_outer"};
      params.set<std::vector<libMesh::Point>>("extra_positions") = {libMesh::Point(0, 0, 0)};
      params.set<std::vector<unsigned int>>("extra_positions_mg_indices") = {0};
    }
    params.set<bool>("use_auto_area_func") = true;
    params.set<bool>("verify_holes") = false;
    params.set<MooseEnum>("boundary_type") = (_mesh_geometry == "Hex") ? "HEXAGON" : "CARTESIAN";
    params.set<unsigned int>("boundary_sectors") =
        getReactorParam<unsigned int>(RGMB::num_sectors_flexible_stitching);
    params.set<Real>("boundary_size") = getReactorParam<Real>(RGMB::assembly_pitch);
    params.set<boundary_id_type>("external_boundary_id") = RGMB::PIN_BOUNDARY_ID_START + _pin_type;
    params.set<BoundaryName>("external_boundary_name") =
        RGMB::ASSEMBLY_BOUNDARY_NAME_PREFIX + std::to_string(_pin_type);
    params.set<SubdomainName>("background_subdomain_name") =
        outermost_block_name + RGMB::TRI_BLOCK_NAME_SUFFIX;
    params.set<unsigned short>("background_subdomain_id") = RGMB::PIN_BLOCK_ID_TRI_FLEXIBLE;

    addMeshSubgenerator("FlexiblePatternGenerator", name() + "_fpg", params);
  }
  {
    // Delete extra boundary created by FlexiblePatternGenerator
    auto params = _app.getFactory().getValidParams("BoundaryDeletionGenerator");

    params.set<MeshGeneratorName>("input") = name() + "_fpg";
    std::vector<BoundaryName> boundaries_to_delete = {};
    if (!has_single_mesh_interval)
      boundaries_to_delete.push_back(std::to_string(1));
    params.set<std::vector<BoundaryName>>("boundary_names") = boundaries_to_delete;

    addMeshSubgenerator("BoundaryDeletionGenerator", name() + "_fpg_delbds", params);
  }
}

void
PinMeshGenerator::generateMetadata()
{
  // Store pin region ids and block names for id swap after extrusion if needed
  // by future mesh generators
  std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>> region_id_map{
      {_pin_type, _region_ids}};

  // Declare mesh properties that need to be moved up to the assembly level
  if (_is_assembly)
  {
    declareMeshProperty(RGMB::assembly_type, _pin_type);
    declareMeshProperty(RGMB::background_block_name, std::vector<std::string>());
    declareMeshProperty(RGMB::duct_block_names, std::vector<std::vector<std::string>>());
    declareMeshProperty(RGMB::is_single_pin, _is_assembly);
    declareMeshProperty(RGMB::is_control_drum, false);
    // Following metadata is only relevant if an output mesh is generated by RGMB
    // because it pertains to region & block ids of elements in the output mesh
    if (!getReactorParam<bool>(RGMB::bypass_meshgen))
    {
      std::map<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>> pin_region_id_map;
      pin_region_id_map.insert(
          std::pair<subdomain_id_type, std::vector<std::vector<subdomain_id_type>>>(
              region_id_map.begin()->first, region_id_map.begin()->second));
      declareMeshProperty(RGMB::pin_region_id_map, pin_region_id_map);
      std::map<subdomain_id_type, std::vector<std::vector<std::string>>> pin_block_name_map;
      pin_block_name_map.insert(std::pair<subdomain_id_type, std::vector<std::vector<std::string>>>(
          _pin_type, _block_names));
      declareMeshProperty(RGMB::pin_block_name_map, pin_block_name_map);
    }
  }
  // Declare mesh properties that are only relevant to pin meshes
  else
  {
    declareMeshProperty(RGMB::pin_type, _pin_type);
    declareMeshProperty(RGMB::pin_region_ids, region_id_map);
    declareMeshProperty(RGMB::pin_block_names, _block_names);
  }

  // Set metadata to describe pin attributes
  declareMeshProperty(RGMB::pitch, _pitch);
  declareMeshProperty(RGMB::is_homogenized, _homogenized);
  declareMeshProperty(RGMB::ring_radii, _ring_radii);
  declareMeshProperty(RGMB::duct_halfpitches, _duct_halfpitch);
  declareMeshProperty(RGMB::extruded, _extrude && _mesh_dimensions == 3);

  unsigned int n_axial_levels =
      (_mesh_dimensions == 3)
          ? getReactorParam<std::vector<unsigned int>>(RGMB::axial_mesh_intervals).size()
          : 1;
  std::vector<std::vector<subdomain_id_type>> ring_region_ids(
      n_axial_levels, std::vector<subdomain_id_type>(_ring_radii.size()));
  std::vector<std::vector<subdomain_id_type>> duct_region_ids(
      n_axial_levels, std::vector<subdomain_id_type>(_duct_halfpitch.size()));
  std::vector<subdomain_id_type> background_region_ids(n_axial_levels);

  for (const auto axial_idx : make_range(n_axial_levels))
  {
    for (const auto ring_idx : index_range(_ring_radii))
      ring_region_ids[axial_idx][ring_idx] = _region_ids[axial_idx][ring_idx];

    background_region_ids[axial_idx] = _region_ids[axial_idx][_ring_radii.size()];

    for (unsigned int duct_idx = _ring_radii.size() + 1;
         duct_idx < _duct_halfpitch.size() + _ring_radii.size() + 1;
         ++duct_idx)
      duct_region_ids[axial_idx][duct_idx - _ring_radii.size() - 1] =
          _region_ids[axial_idx][duct_idx];
  }

  // Define mesh properties related to region ids
  declareMeshProperty(RGMB::ring_region_ids, ring_region_ids);
  declareMeshProperty(RGMB::background_region_id, background_region_ids);
  declareMeshProperty(RGMB::duct_region_ids, duct_region_ids);
}

std::unique_ptr<MeshBase>
PinMeshGenerator::generate()
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
  if (hasMeshProperty<Real>("max_radius_meta", name() + "_2D"))
  {
    const auto max_radius_meta = getMeshProperty<Real>("max_radius_meta", name() + "_2D");
    setMeshProperty("max_radius_meta", max_radius_meta);
  }
  if (hasMeshProperty<unsigned int>("background_intervals_meta", name() + "_2D"))
  {
    const auto background_intervals_meta =
        getMeshProperty<unsigned int>("background_intervals_meta", name() + "_2D");
    setMeshProperty("background_intervals_meta", background_intervals_meta);
  }
  if (hasMeshProperty<dof_id_type>("node_id_background_meta", name() + "_2D"))
  {
    const auto node_id_background_meta =
        getMeshProperty<dof_id_type>("node_id_background_meta", name() + "_2D");
    setMeshProperty("node_id_background_meta", node_id_background_meta);
  }

  // This generate() method will be called once the subgenerators that we depend on
  // have been called. This is where we reassign subdomain ids/names according to what
  // the user has provided, and also where we set region_id, pin_type_id, and radial_id
  // extra element integers

  // Add region id, pin type id, and radial id to the mesh as element integers
  std::string region_id_name = "region_id";
  std::string pin_type_id_name = "pin_type_id";
  std::string assembly_type_id_name = "assembly_type_id";
  std::string plane_id_name = "plane_id";
  std::string radial_id_name = "radial_id";
  const std::string default_block_name =
      (_is_assembly ? RGMB::ASSEMBLY_BLOCK_NAME_PREFIX : RGMB::PIN_BLOCK_NAME_PREFIX) +
      std::to_string(_pin_type);

  auto region_id_int = getElemIntegerFromMesh(*(*_build_mesh), region_id_name);
  auto radial_id_int = getElemIntegerFromMesh(*(*_build_mesh), radial_id_name);
  auto pin_type_id_int = getElemIntegerFromMesh(*(*_build_mesh), pin_type_id_name);
  unsigned int plane_id_int = 0;
  unsigned int assembly_type_id_int = 0;
  if (_extrude)
    plane_id_int = getElemIntegerFromMesh(*(*_build_mesh), plane_id_name, true);
  if (_is_assembly)
    assembly_type_id_int = getElemIntegerFromMesh(*(*_build_mesh), assembly_type_id_name);

  // Get next free block ID in mesh in case subdomain ids need to be remapped
  auto next_block_id = MooseMeshUtils::getNextFreeSubdomainID(*(*(_build_mesh)));
  std::map<std::string, SubdomainID> rgmb_name_id_map;

  // Loop through all elements and set regions ids, pin type id, and radial idx.
  // These element integers are also used to infer the block name for the region,
  // and block IDs/names will be reassigned on the pin mesh if necessary
  for (auto & elem : (*_build_mesh)->active_element_ptr_range())
  {
    const auto base_block_id = elem->subdomain_id();
    const auto base_block_name = (*_build_mesh)->subdomain_name(base_block_id);

    // Check if block name has correct prefix
    std::string prefix = RGMB::PIN_BLOCK_NAME_PREFIX + std::to_string(_pin_type) + "_R";
    if (!(base_block_name.find(prefix, 0) == 0))
      continue;
    // Radial index is integer value of substring after prefix
    std::string radial_str = base_block_name.substr(prefix.length());

    // Filter out RGMB::TRI_BLOCK_NAME_SUFFIX if needed
    const std::string suffix = RGMB::TRI_BLOCK_NAME_SUFFIX;
    const std::size_t found = radial_str.find(suffix);
    if (found != std::string::npos)
      radial_str.replace(found, suffix.length(), "");
    const unsigned int radial_idx = std::stoi(radial_str);

    // Region id is inferred from z_id and radial_idx
    dof_id_type z_id = _extrude ? elem->get_extra_integer(plane_id_int) : 0;
    const subdomain_id_type elem_region_id = _region_ids[std::size_t(z_id)][radial_idx];

    // Set element integers
    elem->set_extra_integer(region_id_int, elem_region_id);
    elem->set_extra_integer(pin_type_id_int, _pin_type);
    elem->set_extra_integer(radial_id_int, radial_idx);
    if (_is_assembly)
      elem->set_extra_integer(assembly_type_id_int, _pin_type);

    // Set element block name and block id
    auto elem_block_name = default_block_name;
    if (_has_block_names)
      elem_block_name += "_" + _block_names[std::size_t(z_id)][radial_idx];
    else if (getReactorParam<bool>(RGMB::region_id_as_block_name))
      elem_block_name += "_REG" + std::to_string(elem_region_id);
    if (elem->type() == TRI3 || elem->type() == PRISM6)
      elem_block_name += RGMB::TRI_BLOCK_NAME_SUFFIX;
    updateElementBlockNameId(
        *(*_build_mesh), elem, rgmb_name_id_map, elem_block_name, next_block_id);
  }

  // Mark mesh as not prepared, as block IDs were re-assigned in this method
  (*_build_mesh)->set_isnt_prepared();

  return std::move(*_build_mesh);
}
