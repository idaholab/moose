//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ControlDrumMeshGenerator.h"

#include "ReactorGeometryMeshBuilderBase.h"
#include "PolygonalMeshGenerationUtils.h"
#include "MooseApp.h"
#include "Factory.h"
#include "libmesh/elem.h"
#include "MooseMeshUtils.h"
#include "PolygonMeshGeneratorBase.h"

registerMooseObject("ReactorApp", ControlDrumMeshGenerator);

InputParameters
ControlDrumMeshGenerator::validParams()
{
  auto params = ReactorGeometryMeshBuilderBase::validParams();

  params.addRequiredParam<MeshGeneratorName>(
      "reactor_params",
      "The ReactorMeshParams MeshGenerator that is the basis for this component mesh.");
  params.addRequiredParam<subdomain_id_type>(
      "assembly_type",
      "The assembly type integer ID to use for this control drum definition. "
      "This parameter should be uniquely defined for each ControlDrumMeshGenerator "
      "and AssemblyMeshGenerator structure in the RGMB workflow.");
  params.addParam<bool>("extrude",
                        false,
                        "Determines if this is the final step in the geometry construction"
                        " and extrudes the 2D geometry to 3D. If this is true then this mesh "
                        "cannot be used in further mesh building in the Reactor workflow");
  params.addRequiredRangeCheckedParam<Real>(
      "drum_inner_radius", "drum_inner_radius>0", "Inner radius of drum region");
  params.addRequiredRangeCheckedParam<Real>(
      "drum_outer_radius", "drum_outer_radius>0", "Outer radius of drum region");
  params.addRangeCheckedParam<unsigned int>(
      "drum_inner_intervals",
      1,
      "drum_inner_intervals>0",
      "Number of radial mesh intervals in region up to inner drum radius");
  params.addRangeCheckedParam<unsigned int>(
      "drum_intervals", 1, "drum_intervals>0", "Number of radial mesh intervals in drum region");
  params.addRangeCheckedParam<Real>("pad_start_angle",
                                    "pad_start_angle>=0 & pad_start_angle < 360",
                                    "Starting angle of drum pad region");
  params.addRangeCheckedParam<Real>(
      "pad_end_angle", "pad_end_angle>0 & pad_end_angle < 720", "Ending angle of drum pad region");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_azimuthal_sectors",
      "num_azimuthal_sectors>2",
      "Number of azimuthal sectors to sub-divide the drum region into");
  params.addRequiredParam<std::vector<std::vector<subdomain_id_type>>>(
      "region_ids",
      "IDs for each radial and axial zone for assignment of region_id extra element "
      "id. "
      "Inner indexing is radial zones (drum inner/drum/drum outer), outer indexing is axial");
  params.addParam<std::vector<std::vector<std::string>>>(
      "block_names",
      "Block names for each radial and axial zone. "
      "Inner indexing is radial zones (drum inner/drum/drum outer), outer indexing is axial");
  params.addParam<Real>("azimuthal_node_tolerance",
                        0.1,
                        "(in degrees) The absolute tolerance for which to shift an azimuthal node "
                        "to match the pad start/end angles");

  params.addParamNamesToGroup("region_ids assembly_type", "ID assigment");
  params.addParamNamesToGroup("drum_inner_radius drum_outer_radius drum_inner_intervals "
                              "drum_intervals num_azimuthal_sectors",
                              "Drum specifications");
  params.addParamNamesToGroup("pad_start_angle pad_end_angle azimuthal_node_tolerance",
                              "Control pad specifications");

  params.addClassDescription(
      "This ControlDrumMeshGenerator object is designed to generate "
      "drum-like structures, with IDs, from a reactor geometry. "
      "These structures can be used directly within CoreMeshGenerator to stitch"
      "control drums into a core lattice alongside AssemblyMeshGenerator structures");
  // depletion id generation params are added
  addDepletionIDParams(params);

  return params;
}

ControlDrumMeshGenerator::ControlDrumMeshGenerator(const InputParameters & parameters)
  : ReactorGeometryMeshBuilderBase(parameters),
    _assembly_type(getParam<subdomain_id_type>("assembly_type")),
    _drum_inner_radius(getParam<Real>("drum_inner_radius")),
    _drum_outer_radius(getParam<Real>("drum_outer_radius")),
    _extrude(getParam<bool>("extrude")),
    _region_ids(getParam<std::vector<std::vector<subdomain_id_type>>>("region_ids"))
{
  // Initialize ReactorMeshParams object
  initializeReactorMeshParams(getParam<MeshGeneratorName>("reactor_params"));

  // Flexible stitching needs to be invoked in order to create control drum mesh
  if (!getReactorParam<bool>(RGMB::flexible_assembly_stitching))
    mooseError("'flexible_assembly_stitching' needs to be set to true in ReactorMeshParams in "
               "order to use ControlDrumMeshGenerator");

  _geom_type = getReactorParam<std::string>(RGMB::mesh_geometry);
  _mesh_dimensions = getReactorParam<unsigned int>(RGMB::mesh_dimensions);

  const auto drum_inner_intervals = getParam<unsigned int>("drum_inner_intervals");
  const auto drum_intervals = getParam<unsigned int>("drum_intervals");
  const auto num_sectors = getParam<unsigned int>("num_azimuthal_sectors");
  const auto assembly_pitch = getReactorParam<Real>(RGMB::assembly_pitch);

  // Check drum pad parameters
  if (isParamSetByUser("pad_start_angle"))
  {
    _pad_start_angle = getParam<Real>("pad_start_angle");
    if (!isParamSetByUser("pad_end_angle"))
      paramError("pad_start_angle",
                 "If 'pad_start_angle' is set, 'pad_end_angle' needs to also be set.");
    _pad_end_angle = getParam<Real>("pad_end_angle");

    if ((_pad_end_angle - _pad_start_angle >= 360) || (_pad_end_angle - _pad_start_angle <= 0))
      paramError("pad_start_angle",
                 "The difference between 'pad_end_angle' and 'pad_start_angle' must be between 0 "
                 "and 360 exclusive.");
    _has_pad_region = true;
  }
  else
  {
    if (isParamSetByUser("pad_end_angle"))
      paramError("pad_end_angle",
                 "If 'pad_end_angle' is set, 'pad_start_angle' needs to also be set.");
    _has_pad_region = false;
  }

  // Error checking for azimuthal node tolerance
  const auto azimuthal_node_tolerance = getParam<Real>("azimuthal_node_tolerance");
  if (!_has_pad_region && isParamSetByUser("azimuthal_node_tolerance"))
    paramError("azimuthal_node_tolerance",
               "This parameter is relevant only when pad start and end angles are defined");
  if (_has_pad_region && MooseUtils::absoluteFuzzyGreaterEqual(2. * azimuthal_node_tolerance,
                                                               360. / (Real)num_sectors))
    paramError("azimuthal_node_tolerance",
               "Azimuthal node tolerance should be smaller than half the azimuthal interval size "
               "as defined by 'num_azimuthal_sectors'");
  if (_has_pad_region && MooseUtils::absoluteFuzzyGreaterEqual(2. * azimuthal_node_tolerance,
                                                               _pad_end_angle - _pad_start_angle))
    paramError("azimuthal_node_tolerance",
               "Azimuthal node tolerance should be smaller than half the difference of the pad "
               "angle range");

  // Check region IDs have correct size
  const unsigned int n_radial_regions = _has_pad_region ? 4 : 3;
  unsigned int n_axial_levels =
      (_mesh_dimensions == 3)
          ? getReactorParam<std::vector<unsigned int>>(RGMB::axial_mesh_intervals).size()
          : 1;
  if (_region_ids.size() != n_axial_levels)
    mooseError("The size of region IDs must be equal to the number of axial levels as defined in "
               "the ReactorMeshParams object");
  if (_region_ids[0].size() != n_radial_regions)
  {
    std::string err_msg =
        "'region_ids' parameter does not have the correct number of elements per axial zone. ";
    err_msg += _has_pad_region
                   ? "For control drums with a pad region, 4 radial IDs need to be provided per "
                     "axial zone (drum inner, drum pad, drum ex-pad, and drum outer)"
                   : "For control drums with no pad region, 3 radial IDs need to be provided per "
                     "axial zone (drum inner, drum, and drum outer)";
    paramError("region_ids", err_msg);
  }

  // Check block names have the correct size
  if (isParamValid("block_names"))
  {
    if (getReactorParam<bool>(RGMB::region_id_as_block_name))
      paramError("block_names",
                 "If ReactorMeshParams/region_id_as_block_name is set, block_names should not be "
                 "specified in ControlDrumMeshGenerator");
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

  // Check extrusion parameters
  if (_extrude && _mesh_dimensions != 3)
    paramError("extrude",
               "In order to extrude this mesh, ReactorMeshParams/dim needs to be set to 3\n");
  if (_extrude && (!hasReactorParam<boundary_id_type>(RGMB::top_boundary_id) ||
                   !hasReactorParam<boundary_id_type>(RGMB::bottom_boundary_id)))
    mooseError("Both top_boundary_id and bottom_boundary_id must be provided in ReactorMeshParams "
               "if using extruded geometry");

  // Define azimuthal angles explicitly based on num_azimuthal_sectors and manually add pad start
  // angle and end angle if they are not contained within these angle intervals
  std::vector<Real> azimuthal_angles;
  const auto custom_start_angle = _has_pad_region ? _pad_start_angle : 0.;
  const auto custom_end_angle =
      _has_pad_region ? ((_pad_end_angle > 360) ? _pad_end_angle - 360. : _pad_end_angle) : 0.;
  for (unsigned int i = 0; i < num_sectors; ++i)
  {
    Real current_azim_angle = (Real)i * 360. / (Real)num_sectors;
    Real next_azim_angle = (Real)(i + 1) * 360. / (Real)num_sectors;
    if (!_has_pad_region)
      azimuthal_angles.push_back(current_azim_angle);
    else
    {
      // When pad regions are involved, check if the current azimuthal node angle coincides with
      // pad start/end angle to within tolerance. If it does, then shift the azimuthal node location
      // to match the pad angle location. If it does not and the pad angle falls within the
      // azimuthal sector, then create an additional node for the pad location
      if (MooseUtils::absoluteFuzzyLessEqual(std::abs(current_azim_angle - custom_start_angle),
                                             azimuthal_node_tolerance))
      {
        azimuthal_angles.push_back(custom_start_angle);
      }
      else if (MooseUtils::absoluteFuzzyLessEqual(std::abs(current_azim_angle - custom_end_angle),
                                                  azimuthal_node_tolerance))
      {
        azimuthal_angles.push_back(custom_end_angle);
      }
      else if (MooseUtils::absoluteFuzzyGreaterThan(custom_start_angle - current_azim_angle,
                                                    azimuthal_node_tolerance) &&
               MooseUtils::absoluteFuzzyGreaterThan(next_azim_angle - custom_start_angle,
                                                    azimuthal_node_tolerance))
      {
        mooseWarning("pad_start_angle not contained within drum azimuthal discretization so "
                     "additional azimuthal nodes are created to align mesh with this angle");
        azimuthal_angles.push_back(current_azim_angle);
        azimuthal_angles.push_back(custom_start_angle);
      }
      else if (MooseUtils::absoluteFuzzyGreaterThan(custom_end_angle - current_azim_angle,
                                                    azimuthal_node_tolerance) &&
               MooseUtils::absoluteFuzzyGreaterThan(next_azim_angle - custom_end_angle,
                                                    azimuthal_node_tolerance))
      {
        mooseWarning("pad_end_angle not contained within drum azimuthal discretization so "
                     "additional azimuthal nodes are created to align mesh with this angle");
        azimuthal_angles.push_back(current_azim_angle);
        azimuthal_angles.push_back(custom_end_angle);
      }
      else
        azimuthal_angles.push_back(current_azim_angle);
    }
  }

  // Check drum radius parameters
  if (_drum_inner_radius >= _drum_outer_radius)
    paramError("drum_outer_radius", "Drum outer radius must be larger than the inner radius");
  // Check if volume preserved radius of outer radius exceeds assembly halfpitch. In data driven
  // mode, radius is assumed not to need volume preservation
  auto radius_corrfac =
      getReactorParam<bool>(RGMB::bypass_meshgen)
          ? 1.0
          : PolygonalMeshGenerationUtils::radiusCorrectionFactor(azimuthal_angles);
  if (_drum_outer_radius * radius_corrfac >= assembly_pitch / 2.)
    paramError("drum_outer_radius",
               "Volume-corrected outer radius of drum region must be smaller than half the "
               "assembly pitch as "
               "defined by 'ReactorMeshParams/assembly_pitch'");

  // No subgenerators will be called if option to bypass mesh generators is enabled
  if (!getReactorParam<bool>(RGMB::bypass_meshgen))
  {
    const std::string block_name_prefix =
        RGMB::DRUM_BLOCK_NAME_PREFIX + std::to_string(_assembly_type);

    {
      // Invoke AdvancedConcentricCircleGenerator to define drum mesh without background region
      auto params = _app.getFactory().getValidParams("AdvancedConcentricCircleGenerator");

      params.set<std::vector<Real>>("customized_azimuthal_angles") = azimuthal_angles;
      params.set<std::vector<double>>("ring_radii") = {_drum_inner_radius, _drum_outer_radius};
      params.set<std::vector<unsigned int>>("ring_intervals") = {drum_inner_intervals,
                                                                 drum_intervals};
      if (drum_inner_intervals > 1)
      {
        params.set<std::vector<subdomain_id_type>>("ring_block_ids") = {
            RGMB::CONTROL_DRUM_BLOCK_ID_INNER_TRI,
            RGMB::CONTROL_DRUM_BLOCK_ID_INNER,
            RGMB::CONTROL_DRUM_BLOCK_ID_PAD};
        params.set<std::vector<SubdomainName>>("ring_block_names") = {
            block_name_prefix + "_R0_TRI", block_name_prefix + "_R0", block_name_prefix + "_R1"};
      }
      else
      {
        params.set<std::vector<subdomain_id_type>>("ring_block_ids") = {
            RGMB::CONTROL_DRUM_BLOCK_ID_INNER, RGMB::CONTROL_DRUM_BLOCK_ID_PAD};
        params.set<std::vector<SubdomainName>>("ring_block_names") = {block_name_prefix + "_R0",
                                                                      block_name_prefix + "_R1"};
      }
      params.set<bool>("create_outward_interface_boundaries") = false;

      addMeshSubgenerator("AdvancedConcentricCircleGenerator", name() + "_accg", params);
    }
    {
      // Invoke FlexiblePatternGenerator to triangulate drum background region
      auto params = _app.getFactory().getValidParams("FlexiblePatternGenerator");

      params.set<std::vector<MeshGeneratorName>>("inputs") = {name() + "_accg"};
      params.set<std::vector<libMesh::Point>>("extra_positions") = {libMesh::Point(0, 0, 0)};
      params.set<std::vector<unsigned int>>("extra_positions_mg_indices") = {0};
      params.set<bool>("use_auto_area_func") = true;
      // `verify_holes` is set to false to prevent false positive instances of points outside of
      // defined holes, which can create test failures on certain dev environments
      params.set<bool>("verify_holes") = false;
      params.set<MooseEnum>("boundary_type") = (_geom_type == "Hex") ? "HEXAGON" : "CARTESIAN";
      params.set<unsigned int>("boundary_sectors") =
          getReactorParam<unsigned int>(RGMB::num_sectors_flexible_stitching);
      params.set<Real>("boundary_size") = assembly_pitch;
      params.set<boundary_id_type>("external_boundary_id") =
          RGMB::ASSEMBLY_BOUNDARY_ID_START + _assembly_type;
      params.set<BoundaryName>("external_boundary_name") =
          RGMB::ASSEMBLY_BOUNDARY_NAME_PREFIX + std::to_string(_assembly_type);
      params.set<SubdomainName>("background_subdomain_name") = block_name_prefix + "_R2_TRI";
      params.set<subdomain_id_type>("background_subdomain_id") = RGMB::CONTROL_DRUM_BLOCK_ID_OUTER;

      addMeshSubgenerator("FlexiblePatternGenerator", name() + "_fpg", params);

      // Pass mesh meta-data defined in subgenerator constructor to this MeshGenerator
      copyMeshProperty<bool>("is_control_drum_meta", name() + "_fpg");
      copyMeshProperty<Real>("pattern_pitch_meta", name() + "_fpg");
    }
    std::string build_mesh_name = name() + "_delbds";
    {
      // Invoke BoundaryDeletionGenerator to delete extra sidesets created by
      // AdvancedConcentricCircleGenerator
      auto params = _app.getFactory().getValidParams("BoundaryDeletionGenerator");

      params.set<MeshGeneratorName>("input") = name() + "_fpg";
      params.set<std::vector<BoundaryName>>("boundary_names") = {"0", "2"};

      addMeshSubgenerator("BoundaryDeletionGenerator", build_mesh_name, params);
    }
    if (_extrude && _mesh_dimensions == 3)
      build_mesh_name = callExtrusionMeshSubgenerators(build_mesh_name);

    // Store final mesh subgenerator
    _build_mesh = &getMeshByName(build_mesh_name);
  }

  generateMetadata();
}

void
ControlDrumMeshGenerator::generateMetadata()
{
  // Declare metadata for use in downstream mesh generators
  declareMeshProperty(RGMB::assembly_type, _assembly_type);
  declareMeshProperty(RGMB::pitch, getReactorParam<Real>(RGMB::assembly_pitch));
  declareMeshProperty(RGMB::extruded, _extrude && _mesh_dimensions == 3);
  declareMeshProperty(RGMB::is_control_drum, true);
  declareMeshProperty(RGMB::drum_region_ids, _region_ids);
  declareMeshProperty(RGMB::drum_block_names, _block_names);
  std::vector<Real> drum_pad_angles =
      _has_pad_region ? std::vector<Real>({_pad_start_angle, _pad_end_angle}) : std::vector<Real>();
  declareMeshProperty(RGMB::drum_pad_angles, drum_pad_angles);
  std::vector<Real> drum_radii = std::vector<Real>({_drum_inner_radius, _drum_outer_radius});
  declareMeshProperty(RGMB::drum_radii, drum_radii);
  declareMeshProperty(RGMB::is_homogenized, false);
  declareMeshProperty(RGMB::is_single_pin, false);
}

std::unique_ptr<MeshBase>
ControlDrumMeshGenerator::generate()
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
  // called. This is where we reassign subdomain ids/name in case they were merged when
  // stitching pins into an assembly. This is also where we set region_id and
  // assembly_type_id element integers.

  // Define all extra element names and integers
  std::string plane_id_name = "plane_id";
  std::string region_id_name = "region_id";
  std::string pin_type_id_name = "pin_type_id";
  std::string assembly_type_id_name = "assembly_type_id";
  const std::string default_block_name =
      RGMB::DRUM_BLOCK_NAME_PREFIX + std::to_string(_assembly_type);

  auto pin_type_id_int = getElemIntegerFromMesh(*(*_build_mesh), pin_type_id_name);
  auto region_id_int = getElemIntegerFromMesh(*(*_build_mesh), region_id_name);
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
    const dof_id_type z_id = _extrude ? elem->get_extra_integer(plane_id_int) : 0;

    // Assembly peripheral element (background / duct), set subdomains according
    // to user preferences and set pin type id to RGMB::MAX_PIN_TYPE_ID - peripheral index
    // Region id is inferred from z_id and peripheral_idx
    const auto base_block_id = elem->subdomain_id();
    const auto base_block_name = (*_build_mesh)->subdomain_name(base_block_id);

    // Check if block name has correct prefix
    std::string prefix = RGMB::DRUM_BLOCK_NAME_PREFIX + std::to_string(_assembly_type) + "_R";
    if (!(base_block_name.find(prefix, 0) == 0))
      continue;

    // Radial index is integer value of substring after prefix
    const unsigned int radial_idx = std::stoi(base_block_name.substr(prefix.length()));

    // Drum index distinguishes between elements in pad and ex-pad regions that have the same radial
    // index
    const unsigned int drum_idx =
        getDrumIdxFromRadialIdx(radial_idx, elem->true_centroid()(0), elem->true_centroid()(1));
    subdomain_id_type pin_type = RGMB::MAX_PIN_TYPE_ID - drum_idx;
    elem->set_extra_integer(pin_type_id_int, pin_type);

    const auto elem_rid = _region_ids[z_id][drum_idx];
    elem->set_extra_integer(region_id_int, elem_rid);

    // Set element block name and block id
    auto elem_block_name = default_block_name;
    if (getReactorParam<bool>(RGMB::region_id_as_block_name))
      elem_block_name += "_REG" + std::to_string(elem_rid);
    else if (_has_block_names)
      elem_block_name += "_" + _block_names[z_id][drum_idx];
    if (elem->type() == TRI3 || elem->type() == PRISM6)
      elem_block_name += RGMB::TRI_BLOCK_NAME_SUFFIX;
    updateElementBlockNameId(
        *(*_build_mesh), elem, rgmb_name_id_map, elem_block_name, next_block_id);
  }

  if (getParam<bool>("generate_depletion_id"))
  {
    const MooseEnum option = getParam<MooseEnum>("depletion_id_type");
    addDepletionId(*(*_build_mesh), option, DepletionIDGenerationLevel::Drum, _extrude);
  }

  // Mark mesh as not prepared, as block ID's were re-assigned in this method
  (*_build_mesh)->set_isnt_prepared();

  return std::move(*_build_mesh);
}

unsigned int
ControlDrumMeshGenerator::getDrumIdxFromRadialIdx(const unsigned int radial_idx,
                                                  const Real elem_x,
                                                  const Real elem_y)
{
  // By default, drum index will match radial index unless a pad region is defined
  unsigned int drum_idx = radial_idx;
  if (_has_pad_region)
  {
    if (radial_idx == 1)
    {
      // This is an element in the drum region, use drum angle to determine whether it belongs to
      // pad or ex-pad region Note: drum angle of 0 degrees starts in positive y-direction and
      // increments in clockwise direction, which is consistent
      //       with how AdvancedConcentricCircleMeshGenerator defines azimuthal angles
      Real drum_angle = std::atan2(elem_x, elem_y) * 180. / M_PI;
      if (drum_angle < 0)
        drum_angle += 360;

      // If _pad_end_angle does not exceed 360 degrees, check if drum angle lies within
      // _pad_start_angle and _pad_end_angle. Drum index needs to be incremented if element in
      // ex-pad region
      if (_pad_end_angle <= 360)
      {
        if ((drum_angle < _pad_start_angle) || (drum_angle > _pad_end_angle))
          ++drum_idx;
      }
      else
      {
        // If _pad_end_angle exceeds 360 degrees, check two intervals - _pad_start_angle to 360, and
        // 0 to _pad_end_angle - 360 Drum index needs to be incremented if element in ex-pad region
        if ((drum_angle < _pad_start_angle) && (drum_angle > _pad_end_angle - 360.))
          ++drum_idx;
      }
    }
    else if (radial_idx == 2)
    {
      // Element is in outer drum region, drum index needs to be incremented to account for presence
      // of ex-pad region
      ++drum_idx;
    }
  }
  return drum_idx;
}
