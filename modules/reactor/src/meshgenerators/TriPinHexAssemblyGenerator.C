//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriPinHexAssemblyGenerator.h"
#include "libmesh/mesh_smoother_laplace.h"

#include <cmath>

registerMooseObject("ReactorApp", TriPinHexAssemblyGenerator);

InputParameters
TriPinHexAssemblyGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_sectors_per_side", "num_sectors_per_side>0", "Number of azimuthal sectors per side.");
  params.addRangeCheckedParam<unsigned int>(
      "background_intervals",
      1,
      "background_intervals>0",
      "Number of radial meshing intervals in background "
      "region (region around the rings/pins in the assembly).");
  params.addRangeCheckedParam<std::vector<subdomain_id_type>>(
      "background_block_ids",
      "background_block_ids>0",
      "Optional block ids for the background regions in the pins.");
  params.addParam<std::vector<SubdomainName>>(
      "background_block_names", "Optional block names for the background regions in the pins.");
  params.addRangeCheckedParam<std::vector<std::vector<Real>>>(
      "ring_radii", "ring_radii>0", "Radii of the three sets of major concentric circles (pins).");
  params.addRangeCheckedParam<std::vector<std::vector<unsigned int>>>(
      "ring_intervals",
      "ring_intervals>0",
      "Number of radial mesh intervals within each set of major concentric circles (pins).");
  params.addRangeCheckedParam<std::vector<std::vector<subdomain_id_type>>>(
      "ring_block_ids", "ring_block_ids>0", "Optional block ids for the ring (pin) regions.");
  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "ring_block_names", "Optional block names for the ring (pin) regions.");
  MooseEnum hexagon_size_style("apothem radius", "radius");
  params.addParam<MooseEnum>("hexagon_size_style",
                             hexagon_size_style,
                             "Style in which hexagon size is given (apothem = center to face, "
                             "radius = center to vertex).");
  params.addRequiredRangeCheckedParam<Real>(
      "hexagon_size", "hexagon_size>0", "Size parameter of the hexagon assembly to be generated.");
  params.addParam<Real>(
      "ring_offset", 0.0, "Offset of the ring (pin) center, shared by all three.");
  params.addParam<bool>(
      "preserve_volumes",
      true,
      "Volume of concentric circles (pins) can be preserved using this function.");
  MooseEnum assembly_orientation("pin_up pin_down", "pin_up");
  params.addParam<MooseEnum>(
      "assembly_orientation", assembly_orientation, "Orientation of the generated assembly.");
  params.addRangeCheckedParam<boundary_id_type>("external_boundary_id",
                                                "external_boundary_id>0",
                                                "Optional customized external boundary id.");
  params.addParam<std::string>("external_boundary_name",
                               "Optional customized external boundary name.");
  params.addParam<std::string>(
      "pin_id_name", "Name of extra integer ID to be assigned to each of the three pin domains.");
  params.addParam<std::vector<dof_id_type>>(
      "pin_id_values",
      "Values of extra integer ID to be assigned to each of the three pin domains.");
  params.addParamNamesToGroup("ring_block_ids ring_block_names background_block_ids "
                              "background_block_names external_boundary_id external_boundary_name",
                              "Customized Subdomain/Boundary ids/names");
  addRingAndSectorIDParams(params);
  params.addClassDescription(
      "This TriPinHexAssemblyGenerator object generates a hexagonal assembly "
      "mesh with three circular pins in a triangle at the center.");
  return params;
}

TriPinHexAssemblyGenerator::TriPinHexAssemblyGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _ring_radii(isParamValid("ring_radii")
                    ? (getParam<std::vector<std::vector<Real>>>("ring_radii").size() == 1
                           ? std::vector<std::vector<Real>>(
                                 3, getParam<std::vector<std::vector<Real>>>("ring_radii").front())
                           : getParam<std::vector<std::vector<Real>>>("ring_radii"))
                    : std::vector<std::vector<Real>>(3, std::vector<Real>())),
    _ring_intervals(
        isParamValid("ring_intervals")
            ? (getParam<std::vector<std::vector<unsigned int>>>("ring_intervals").size() == 1
                   ? std::vector<std::vector<unsigned int>>(
                         3,
                         getParam<std::vector<std::vector<unsigned int>>>("ring_intervals").front())
                   : getParam<std::vector<std::vector<unsigned int>>>("ring_intervals"))
            : std::vector<std::vector<unsigned int>>(3, std::vector<unsigned int>())),
    _ring_block_ids(
        isParamValid("ring_block_ids")
            ? (getParam<std::vector<std::vector<subdomain_id_type>>>("ring_block_ids").size() == 1
                   ? std::vector<std::vector<subdomain_id_type>>(
                         3,
                         getParam<std::vector<std::vector<subdomain_id_type>>>("ring_block_ids")
                             .front())
                   : getParam<std::vector<std::vector<subdomain_id_type>>>("ring_block_ids"))
            : std::vector<std::vector<subdomain_id_type>>(3, std::vector<subdomain_id_type>())),
    _ring_block_names(
        isParamValid("ring_block_names")
            ? (getParam<std::vector<std::vector<SubdomainName>>>("ring_block_names").size() == 1
                   ? std::vector<std::vector<SubdomainName>>(
                         3,
                         getParam<std::vector<std::vector<SubdomainName>>>("ring_block_names")
                             .front())
                   : getParam<std::vector<std::vector<SubdomainName>>>("ring_block_names"))
            : std::vector<std::vector<SubdomainName>>(3, std::vector<SubdomainName>())),
    _hexagon_size_style(
        getParam<MooseEnum>("hexagon_size_style").template getEnum<PolygonSizeStyle>()),
    _side_length(_hexagon_size_style == PolygonSizeStyle::radius
                     ? getParam<Real>("hexagon_size")
                     : (getParam<Real>("hexagon_size") / cos(M_PI / 6.0))),
    _ring_offset(getParam<Real>("ring_offset")),
    _preserve_volumes(getParam<bool>("preserve_volumes")),
    _assembly_orientation(
        getParam<MooseEnum>("assembly_orientation").template getEnum<AssmOrient>()),
    _num_sectors_per_side(getParam<unsigned int>("num_sectors_per_side")),
    _background_intervals(getParam<unsigned int>("background_intervals")),
    _background_block_ids(isParamValid("background_block_ids")
                              ? getParam<std::vector<subdomain_id_type>>("background_block_ids")
                              : std::vector<subdomain_id_type>()),
    _background_block_names(isParamValid("background_block_names")
                                ? getParam<std::vector<SubdomainName>>("background_block_names")
                                : std::vector<SubdomainName>()),
    _external_boundary_id(isParamValid("external_boundary_id")
                              ? getParam<boundary_id_type>("external_boundary_id")
                              : 0),
    _external_boundary_name(isParamValid("external_boundary_name")
                                ? getParam<std::string>("external_boundary_name")
                                : std::string()),
    _pin_id_name(isParamValid("pin_id_name") ? getParam<std::string>("pin_id_name")
                                             : std::string()),
    _pin_id_values(isParamValid("pin_id_values")
                       ? getParam<std::vector<dof_id_type>>("pin_id_values")
                       : std::vector<dof_id_type>()),
    _node_id_background_meta(declareMeshProperty<dof_id_type>("node_id_background_meta", 0))
{
  declareMeshProperty<Real>("pitch_meta", _side_length * std::sqrt(3.0));
  declareMeshProperty<Real>("pattern_pitch_meta", _side_length * std::sqrt(3.0));
  declareMeshProperty<bool>("is_control_drum_meta", false);
  declareMeshProperty<unsigned int>("background_intervals_meta", _background_intervals);
  declareMeshProperty<Real>("max_radius_meta", 0.0);
  declareMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta",
                                                 {_num_sectors_per_side,
                                                  _num_sectors_per_side,
                                                  _num_sectors_per_side,
                                                  _num_sectors_per_side,
                                                  _num_sectors_per_side,
                                                  _num_sectors_per_side});

  /* Parameter checks */
  if (_ring_radii.size() != 3)
    paramError("ring_radii", "This parameter must have a size of one or three if provided.");
  else
    _has_rings = {!_ring_radii[0].empty(), !_ring_radii[1].empty(), !_ring_radii[2].empty()};

  if (_ring_intervals.size() != 3)
    paramError("ring_intervals", "This parameter must have a size of one or three.");
  else
    for (unsigned int i = 0; i < 3; i++)
      if (_ring_intervals[i].size() != _ring_radii[i].size())
        paramError("ring_intervals", "The parameter must be consistent with ring_radii.");
  if (_ring_block_ids.size() != 3)
    paramError("ring_block_ids", "This parameter must have a size of one or three if provided.");
  else
    for (unsigned int i = 0; i < 3; i++)
    {
      if (!_has_rings[i] && !_ring_block_ids[i].empty())
        paramError("ring_block_ids",
                   "The parameter must be consistent with ring_radii if provided.");
      else if (!_ring_block_ids[i].empty() &&
               _ring_block_ids[i].size() != (_ring_radii[i].size() + (_ring_intervals[i][0] > 1)))
        paramError("ring_block_ids",
                   "The parameter must be consistent with ring_radii if provided.");
      if (!_has_rings[i] && !_ring_block_names[i].empty())
        paramError("ring_block_names",
                   "The parameter must be consistent with ring_radii if provided.");
      else if (!_ring_block_names[i].empty() &&
               _ring_block_names[i].size() != (_ring_radii[i].size() + (_ring_intervals[i][0] > 1)))
        paramError("ring_block_names",
                   "The parameter must be consistent with ring_radii if provided.");
    }
  if (!_background_block_ids.empty())
  {
    if (_has_rings[0] && _has_rings[1] && _has_rings[2] && _background_block_ids.size() != 1)
      paramError("background_block_ids",
                 "If provided, the size of this parameter must be one if all sections have rings.");
    else if (!_has_rings[0] && !_has_rings[1] && !_has_rings[2] && _background_intervals == 1 &&
             _background_block_ids.size() != 1)
      paramError("background_block_ids",
                 "If provided, the size of this parameter must be one if no sections have rings "
                 "and background_intervals is one.");
    else if ((!_has_rings[0] || !_has_rings[1] || !_has_rings[2]) &&
             _background_block_ids.size() != 2)
      paramError(
          "background_block_ids",
          "If provided, the size of this parameter must be two if ring-free section exists.");
  }
  if (!_background_block_names.empty())
  {
    if (_has_rings[0] && _has_rings[1] && _has_rings[2] && _background_block_names.size() != 1)
      paramError("background_block_names",
                 "If provided, the size of this parameter must be one if all sections have rings.");
    else if (!_has_rings[0] && !_has_rings[1] && !_has_rings[2] && _background_intervals == 1 &&
             _background_block_names.size() != 1)
      paramError("background_block_names",
                 "If provided, the size of this parameter must be one if no sections have rings "
                 "and background_intervals is one.");
    else if ((!_has_rings[0] || !_has_rings[1] || !_has_rings[2]) &&
             _background_block_names.size() != 2)
      paramError(
          "background_block_names",
          "If provided, the size of this parameter must be two if ring-free section exists.");
  }
  if (_pin_id_name.empty())
  {
    if (!_pin_id_values.empty())
      paramError("pin_id_values",
                 "This parameter cannot be used when pin_id_name is not provided.");
  }
  else if (_pin_id_values.size() != 3)
    paramError(
        "pin_id_values",
        "If pin_id_name is provided, this parameter must be provided with a length of three.");

  // Just perform a simple and straightforward check for `ring_offset` here.
  // A more comprehensive check for both `ring_offset` and `ring_radii` is done later.
  if (std::abs(_ring_offset) >= _side_length / 2.0)
    paramError(
        "ring_offset",
        "This parameter cannot translate the ring center out of the hexagon assembly region.");
}

std::unique_ptr<MeshBase>
TriPinHexAssemblyGenerator::generate()
{
  /* Pair specified block names and ids */
  std::set<subdomain_id_type> tmp_block_ids;
  std::set<SubdomainName> tmp_block_names;
  std::vector<std::pair<subdomain_id_type, SubdomainName>> block_info;
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < _ring_block_names[i].size(); j++)
    {
      tmp_block_names.emplace(_ring_block_names[i][j]);
      tmp_block_ids.emplace(_ring_block_ids[i].empty() ? (j + 1) : _ring_block_ids[i][j]);
      if (tmp_block_names.size() != tmp_block_ids.size())
        paramError("ring_block_names",
                   "The block name assignment must be compatible with the existing block ids.");
      else
        block_info.push_back(std::make_pair(
            _ring_block_ids[i].empty() ? (j + 1) : _ring_block_ids[i][j], _ring_block_names[i][j]));
    }
  const subdomain_id_type max_ring_radii_size =
      std::max({_ring_radii[0].size(), _ring_radii[1].size(), _ring_radii[2].size()});
  for (unsigned int i = 0; i < _background_block_names.size(); i++)
  {
    tmp_block_names.emplace(_background_block_names[i]);
    tmp_block_ids.emplace(_background_block_ids.empty() ? (max_ring_radii_size + i)
                                                        : _background_block_ids[i]);
    if (tmp_block_names.size() != tmp_block_ids.size())
      paramError("background_block_names",
                 "The block name assignment must be compatible with the existing block ids.");
    else
      block_info.push_back(std::make_pair(_background_block_ids.empty() ? (max_ring_radii_size + i)
                                                                        : _background_block_ids[i],
                                          _background_block_names[i]));
  }
  std::vector<std::vector<subdomain_id_type>> block_ids_new(3, std::vector<subdomain_id_type>());
  for (unsigned int i = 0; i < 3; i++)
  {
    if (_has_rings[i])
    {
      for (unsigned int j = 0; j < (_ring_radii[i].size() + (_ring_intervals[i][0] > 1)); j++)
        block_ids_new[i].push_back(_ring_block_ids[i].empty() ? j + 1 : _ring_block_ids[i][j]);
      block_ids_new[i].push_back(_background_block_ids.empty() ? max_ring_radii_size + 1
                                                               : _background_block_ids.back());
    }
    else
    {
      block_ids_new[i].push_back(_background_block_ids.empty() ? (max_ring_radii_size + 1)
                                                               : _background_block_ids.front());
      block_ids_new[i].push_back(_background_block_ids.empty() ? (max_ring_radii_size + 2)
                                                               : _background_block_ids.back());
    }
  }
  std::vector<std::unique_ptr<ReplicatedMesh>> meshes;
  for (unsigned int i = 0; i < 3; i++)
  {
    meshes.push_back(buildSinglePinSection(_side_length,
                                           _ring_offset,
                                           _ring_radii[i],
                                           _ring_intervals[i],
                                           _has_rings[i],
                                           _preserve_volumes,
                                           _num_sectors_per_side,
                                           _background_intervals,
                                           block_ids_new[i],
                                           _node_id_background_meta));
    // add sector ids
    if (isParamValid("sector_id_name"))
      setSectorExtraIDs(*meshes[i],
                        getParam<std::string>("sector_id_name"),
                        4,
                        std::vector<unsigned int>(4, _num_sectors_per_side));
    // add ring ids
    if (isParamValid("ring_id_name") && _has_rings[i])
      setRingExtraIDs(*meshes[i],
                      getParam<std::string>("ring_id_name"),
                      4,
                      std::vector<unsigned int>(4, _num_sectors_per_side),
                      _ring_intervals[i],
                      getParam<MooseEnum>("ring_id_assign_type") == "ring_wise",
                      false);

    if (!_pin_id_name.empty())
      meshes[i]->add_elem_integer(_pin_id_name, true, _pin_id_values[i]);
    if (i > 0)
    {
      MeshTools::Modification::rotate(*meshes[i], 120.0 * (Real)i, 0, 0);
      meshes[0]->stitch_meshes(*std::move(meshes[i]),
                               OUTER_SIDESET_ID,
                               OUTER_SIDESET_ID,
                               TOLERANCE,
                               /*clear_stitched_boundary_ids=*/true);
    }
  }

  if (_assembly_orientation == AssmOrient::pin_up)
    MeshTools::Modification::rotate(*meshes[0], 90, 0, 0);
  else
    MeshTools::Modification::rotate(*meshes[0], 270, 0, 0);
  /* Add subdomain names */
  for (const auto & block_info_pair : block_info)
    meshes[0]->subdomain_name(block_info_pair.first) = block_info_pair.second;
  if (_external_boundary_id > 0)
    MooseMesh::changeBoundaryId(*meshes[0], OUTER_SIDESET_ID, _external_boundary_id, false);
  if (!_external_boundary_name.empty())
  {
    meshes[0]->get_boundary_info().sideset_name(
        _external_boundary_id > 0 ? _external_boundary_id : (boundary_id_type)OUTER_SIDESET_ID) =
        _external_boundary_name;
    meshes[0]->get_boundary_info().nodeset_name(
        _external_boundary_id > 0 ? _external_boundary_id : (boundary_id_type)OUTER_SIDESET_ID) =
        _external_boundary_name;
  }
  return dynamic_pointer_cast<MeshBase>(meshes[0]);
}

std::unique_ptr<ReplicatedMesh>
TriPinHexAssemblyGenerator::buildSinglePinSection(
    const Real side_length,
    const Real ring_offset,
    const std::vector<Real> ring_radii,
    const std::vector<unsigned int> ring_intervals,
    const bool has_rings,
    const bool preserve_volumes,
    const unsigned int num_sectors_per_side,
    const unsigned int background_intervals,
    const std::vector<subdomain_id_type> block_ids_new,
    dof_id_type & node_id_background_meta)
{
  // Each SinglePinSection, which has a diamond shape, is composed of four general slices.
  // Considering symmetry, two unique general slices (0 and 1) need to be generated.
  // For each slice, the center of the pin is one of its vertices; primary and secondary sides are
  // the two sides that contain the pin center.
  const Real secondary_side_length_0(side_length / 2.0 - ring_offset);
  const Real secondary_side_length_1(
      std::sqrt(side_length * side_length / 4.0 * 3.0 + ring_offset * ring_offset));
  const Real primary_side_length_0(
      std::sqrt(side_length * side_length / 4.0 * 3.0 + ring_offset * ring_offset));
  const Real primary_side_length_1(side_length / 2.0 + ring_offset);
  // Azimuthal angle is the included angle defined by the primary and secondary sides
  const Real azimuthal_angle_0(
      acos((secondary_side_length_0 * secondary_side_length_0 +
            primary_side_length_0 * primary_side_length_0 - side_length * side_length) /
           2.0 / primary_side_length_0 / secondary_side_length_0) /
      M_PI * 180.0);
  const Real azimuthal_angle_1(
      acos((secondary_side_length_1 * secondary_side_length_1 +
            primary_side_length_1 * primary_side_length_1 - side_length * side_length) /
           2.0 / primary_side_length_1 / secondary_side_length_1) /
      M_PI * 180.0);
  // The primary side is parallel to y-axis by default (i.e., rotation_angle is zero). So the
  // general slices need to be rotated before stitching,
  const Real rotation_angle_0(azimuthal_angle_0 - 90.0);
  const Real rotation_angle_1(azimuthal_angle_0 + azimuthal_angle_1 - 90.0);
  // Alpha angle is the other included angle of the slice defined by the primary side and the third
  // side.
  const Real alpha_angle_0(
      acos((primary_side_length_0 * primary_side_length_0 + side_length * side_length -
            secondary_side_length_0 * secondary_side_length_0) /
           2.0 / primary_side_length_0 / side_length) /
      M_PI * 180.0);
  const Real alpha_angle_1(
      acos((primary_side_length_1 * primary_side_length_1 + side_length * side_length -
            secondary_side_length_1 * secondary_side_length_1) /
           2.0 / primary_side_length_1 / side_length) /
      M_PI * 180.0);

  // azimuthal_list is a list of azimuthal intervals of the mesh to support radius correction to
  // ensure preserved volume.
  std::vector<Real> azimuthal_list;
  for (unsigned int i = 0; i < num_sectors_per_side; i++)
  {
    azimuthal_list.push_back(
        atan((Real)i * side_length / num_sectors_per_side * sin(alpha_angle_0 / 180.0 * M_PI) /
             (primary_side_length_0 -
              (Real)i * side_length / num_sectors_per_side * cos(alpha_angle_0 / 180.0 * M_PI))) /
        M_PI * 180.0);
  }
  for (unsigned int i = 0; i < num_sectors_per_side; i++)
  {
    azimuthal_list.push_back(
        azimuthal_angle_0 +
        atan((Real)i * side_length / num_sectors_per_side * sin(alpha_angle_1 / 180.0 * M_PI) /
             (primary_side_length_1 -
              (Real)i * side_length / num_sectors_per_side * cos(alpha_angle_1 / 180.0 * M_PI))) /
            M_PI * 180.0);
  }
  for (unsigned int i = 0; i < num_sectors_per_side * 2; i++)
  {
    azimuthal_list.push_back(azimuthal_list[i] + 180.0);
  }

  std::vector<Real> ring_radii_corr;
  if (has_rings)
  {
    if (preserve_volumes)
    {
      const Real corr_factor = radiusCorrectionFactor(azimuthal_list);
      for (unsigned int i = 0; i < ring_radii.size(); i++)
        ring_radii_corr.push_back(ring_radii[i] * corr_factor);
    }
    else
      ring_radii_corr = ring_radii;
    if (ring_radii_corr.back() > (side_length / 2.0 - std::abs(ring_offset)) * std::sqrt(3.0) / 2.0)
      paramError("ring_radii",
                 "The radii of the rings cannot exceed the boundary of the diamond section.");
  }

  // build the first slice of the polygon.
  auto mesh0 = buildGeneralSlice(
      ring_radii_corr,
      ring_intervals,
      std::vector<Real>(ring_radii_corr.size(), 1.0),
      {std::vector<Real>(ring_radii_corr.size(), 0.0),
       std::vector<Real>(ring_radii_corr.size(), 0.0),
       std::vector<unsigned int>(ring_radii_corr.size(), 0),
       std::vector<Real>(ring_radii_corr.size(), 1.0)},
      {std::vector<Real>(ring_radii_corr.size(), 0.0),
       std::vector<Real>(ring_radii_corr.size(), 0.0),
       std::vector<unsigned int>(ring_radii_corr.size(), 0),
       std::vector<Real>(ring_radii_corr.size(), 1.0)},
      std::vector<Real>(),
      std::vector<unsigned int>(),
      std::vector<Real>(),
      {std::vector<Real>(), std::vector<Real>(), std::vector<unsigned int>(), std::vector<Real>()},
      {std::vector<Real>(), std::vector<Real>(), std::vector<unsigned int>(), std::vector<Real>()},
      primary_side_length_0,
      secondary_side_length_0,
      num_sectors_per_side,
      background_intervals,
      1.0,
      {0.0, 0.0, 0, 1.0},
      {0.0, 0.0, 0, 1.0},
      node_id_background_meta,
      azimuthal_angle_0,
      std::vector<Real>(),
      /* side_index = */ 1,
      false,
      0.0,
      rotation_angle_0,
      false);

  auto mesh1 = buildGeneralSlice(
      ring_radii_corr,
      ring_intervals,
      std::vector<Real>(ring_radii_corr.size(), 1.0),
      {std::vector<Real>(ring_radii_corr.size(), 0.0),
       std::vector<Real>(ring_radii_corr.size(), 0.0),
       std::vector<unsigned int>(ring_radii_corr.size(), 0),
       std::vector<Real>(ring_radii_corr.size(), 1.0)},
      {std::vector<Real>(ring_radii_corr.size(), 0.0),
       std::vector<Real>(ring_radii_corr.size(), 0.0),
       std::vector<unsigned int>(ring_radii_corr.size(), 0),
       std::vector<Real>(ring_radii_corr.size(), 1.0)},
      std::vector<Real>(),
      std::vector<unsigned int>(),
      std::vector<Real>(),
      {std::vector<Real>(), std::vector<Real>(), std::vector<unsigned int>(), std::vector<Real>()},
      {std::vector<Real>(), std::vector<Real>(), std::vector<unsigned int>(), std::vector<Real>()},
      primary_side_length_1,
      secondary_side_length_1,
      num_sectors_per_side,
      background_intervals,
      1.0,
      {0.0, 0.0, 0, 1.0},
      {0.0, 0.0, 0, 1.0},
      node_id_background_meta,
      azimuthal_angle_1,
      std::vector<Real>(),
      /* side_index = */ 1,
      false,
      0.0,
      rotation_angle_1,
      false);

  mesh0->stitch_meshes(*mesh1,
                       SLICE_BEGIN,
                       SLICE_END,
                       TOLERANCE,
                       /*clear_stitched_boundary_ids=*/true);
  MooseMesh::changeBoundaryId(*mesh0, SLICE_BEGIN, SLICE_END, true);

  auto mesh2 = dynamic_pointer_cast<ReplicatedMesh>(mesh0->clone());
  MeshTools::Modification::rotate(*mesh2, 0, 180.0, 0);
  mesh0->stitch_meshes(*mesh2,
                       SLICE_END,
                       SLICE_END,
                       TOLERANCE,
                       /*clear_stitched_boundary_ids=*/true);
  MeshTools::Modification::translate(*mesh0, side_length / 2.0 + ring_offset, 0, 0);

  for (const auto & elem : mesh0->element_ptr_range())
    elem->subdomain_id() = block_ids_new[elem->subdomain_id() - 1];

  return mesh0;
}
