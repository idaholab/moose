//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PatternedHexMeshGenerator.h"
#include "ReportingIDGeneratorUtils.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)
#include <fstream> // used to generate the optional control drum position file

registerMooseObject("ReactorApp", PatternedHexMeshGenerator);

InputParameters
PatternedHexMeshGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  params.addRequiredParam<std::vector<MeshGeneratorName>>("inputs", "The input MeshGenerators.");
  params.addRequiredRangeCheckedParam<std::vector<std::vector<unsigned int>>>(
      "pattern",
      "pattern>=0",
      "A double-indexed hexagonal-shaped array starting with the upper-left corner.");
  MooseEnum pattern_boundary("none hexagon", "hexagon");
  params.addParam<MooseEnum>(
      "pattern_boundary", pattern_boundary, "The boundary shape of the patterned mesh.");
  params.addParam<bool>(
      "generate_core_metadata",
      false,
      "A Boolean parameter that controls whether the core related metadata "
      "is generated for other MOOSE objects such as 'MultiControlDrumFunction' or not.");
  params.addRangeCheckedParam<unsigned int>("background_intervals",
                                            3,
                                            "background_intervals>0",
                                            "Radial intervals in the assembly peripheral region.");
  params.addRangeCheckedParam<Real>("hexagon_size",
                                    "hexagon_size>0.0",
                                    "Size of the outmost hexagon boundary to be generated; this is "
                                    "required only when pattern type is 'hexagon'.");
  MooseEnum hexagon_size_style("apothem radius", "apothem");
  params.addParam<MooseEnum>(
      "hexagon_size_style",
      hexagon_size_style,
      "Style in which the hexagon size is given (default: apothem i.e. half-pitch).");
  params.addRangeCheckedParam<std::vector<Real>>(
      "duct_sizes", "duct_sizes>0.0", "Distance(s) from center to duct(s) inner boundaries.");
  MooseEnum duct_sizes_style("apothem radius", "apothem");
  params.addParam<MooseEnum>("duct_sizes_style",
                             duct_sizes_style,
                             "Style in which hexagon center to duct distance(s) is given (apothem "
                             "= center-to-face, radius = center-to-vertex).");
  params.addRangeCheckedParam<std::vector<unsigned int>>(
      "duct_intervals", "duct_intervals>0", "Number of meshing intervals in each enclosing duct.");
  params.addParam<bool>("uniform_mesh_on_sides",
                        false,
                        "Whether the side elements are reorganized to have a uniform size.");
  params.addParam<bool>("generate_control_drum_positions_file",
                        false,
                        "Whether a positions file is generated in the core mesh mode.");
  params.addParam<bool>("assign_control_drum_id",
                        true,
                        "Whether control drum id is assigned to the mesh as an extra integer.");
  std::string position_file_default = "positions_meta.data";
  params.addParam<std::string>(
      "position_file", position_file_default, "Data file name to store control drum positions.");
  // A hexagon pattern_boundary mesh can be used in "inputs" of `PatternedHexMeshGenerator` after a
  // 90-degree rotation.
  params.addParam<Real>(
      "rotate_angle",
      90.0,
      "Rotate the entire patterned mesh by a certain degrees that is defined here.");
  params.addParam<subdomain_id_type>(
      "background_block_id",
      "Optional customized block id for the background block in 'assembly' mode; must be provided "
      "along with 'duct_block_ids' if 'duct_sizes' is provided.");
  params.addParam<SubdomainName>(
      "background_block_name",
      "Optional customized block name for the background block in 'assembly' mode; must be "
      "provided along with 'duct_block_names' if 'duct_sizes' is provided.");
  params.addParam<std::vector<subdomain_id_type>>(
      "duct_block_ids",
      "Optional customized block ids for each duct geometry block in 'assembly' mode; must be "
      "provided along with 'background_block_id'.");
  params.addParam<std::vector<SubdomainName>>(
      "duct_block_names",
      std::vector<SubdomainName>(),
      "Optional customized block names for each duct geometry block in 'assembly' mode; must be "
      "provided along with 'background_block_name'.");
  params.addRangeCheckedParam<boundary_id_type>("external_boundary_id",
                                                "external_boundary_id>0",
                                                "Optional customized external boundary id.");
  params.addParam<bool>("create_inward_interface_boundaries",
                        false,
                        "Whether the inward interface boundary sidesets are created.");
  params.addParam<bool>("create_outward_interface_boundaries",
                        true,
                        "Whether the outward interface boundary sidesets are created.");
  params.addParam<BoundaryName>(
      "external_boundary_name", BoundaryName(), "Optional customized external boundary name.");
  params.addParam<bool>("deform_non_circular_region",
                        true,
                        "Whether the non-circular region (outside the rings) can be deformed.");
  params.addParam<std::vector<std::string>>("id_name", "List of extra integer ID set names");
  params.addParam<std::vector<MeshGeneratorName>>(
      "exclude_id", "Name of input meshes to be excluded in ID generation");
  std::vector<MooseEnum> option = {MooseEnum("cell pattern manual", "cell")};
  params.addParam<std::vector<MooseEnum>>(
      "assign_type", option, "List of integer ID assignment types");
  params.addParam<std::vector<std::vector<std::vector<dof_id_type>>>>(
      "id_pattern",
      "User-defined element IDs. A double-indexed array starting with the upper-left corner. When "
      "providing multiple patterns, each pattern should be separated using '|'");
  params.addParam<std::vector<std::vector<boundary_id_type>>>(
      "interface_boundary_id_shift_pattern",
      "User-defined shift values for each pattern cell. A double-indexed array starting with the "
      "upper-left corner.");
  MooseEnum quad_elem_type("QUAD4 QUAD8 QUAD9", "QUAD4");
  params.addParam<MooseEnum>(
      "boundary_region_element_type",
      quad_elem_type,
      "Type of the quadrilateral elements to be generated in the boundary region.");
  params.addParamNamesToGroup(
      "background_block_id background_block_name duct_block_ids boundary_region_element_type "
      "duct_block_names external_boundary_id external_boundary_name "
      "create_inward_interface_boundaries create_outward_interface_boundaries",
      "Customized Subdomain/Boundary");
  params.addParamNamesToGroup(
      "generate_control_drum_positions_file assign_control_drum_id position_file", "Control Drum");
  params.addParamNamesToGroup(
      "background_intervals duct_intervals uniform_mesh_on_sides deform_non_circular_region",
      "Mesh Density");
  params.addParamNamesToGroup("id_name exclude_id assign_type id_pattern", "Reporting ID");
  params.addClassDescription(
      "This PatternedHexMeshGenerator source code assembles hexagonal meshes into a hexagonal grid "
      "and optionally forces the outer boundary to be hexagonal and/or adds a duct.");

  return params;
}

PatternedHexMeshGenerator::PatternedHexMeshGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _mesh_ptrs(getMeshes("inputs")),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _pattern(getParam<std::vector<std::vector<unsigned int>>>("pattern")),
    _pattern_boundary(getParam<MooseEnum>("pattern_boundary")),
    _generate_core_metadata(getParam<bool>("generate_core_metadata")),
    _background_intervals(getParam<unsigned int>("background_intervals")),
    _has_assembly_duct(isParamValid("duct_sizes")),
    _duct_sizes(isParamValid("duct_sizes") ? getParam<std::vector<Real>>("duct_sizes")
                                           : std::vector<Real>()),
    _duct_sizes_style(getParam<MooseEnum>("duct_sizes_style").template getEnum<PolygonSizeStyle>()),
    _duct_intervals(isParamValid("duct_intervals")
                        ? getParam<std::vector<unsigned int>>("duct_intervals")
                        : std::vector<unsigned int>()),
    _uniform_mesh_on_sides(getParam<bool>("uniform_mesh_on_sides")),
    _generate_control_drum_positions_file(getParam<bool>("generate_control_drum_positions_file")),
    _assign_control_drum_id(getParam<bool>("assign_control_drum_id")),
    _rotate_angle(getParam<Real>("rotate_angle")),
    _duct_block_ids(isParamValid("duct_block_ids")
                        ? getParam<std::vector<subdomain_id_type>>("duct_block_ids")
                        : std::vector<subdomain_id_type>()),
    _duct_block_names(getParam<std::vector<SubdomainName>>("duct_block_names")),
    _external_boundary_id(isParamValid("external_boundary_id")
                              ? getParam<boundary_id_type>("external_boundary_id")
                              : 0),
    _external_boundary_name(getParam<BoundaryName>("external_boundary_name")),
    _create_inward_interface_boundaries(getParam<bool>("create_inward_interface_boundaries")),
    _create_outward_interface_boundaries(getParam<bool>("create_outward_interface_boundaries")),
    _hexagon_size_style(
        getParam<MooseEnum>("hexagon_size_style").template getEnum<PolygonSizeStyle>()),
    _deform_non_circular_region(getParam<bool>("deform_non_circular_region")),
    _use_reporting_id(isParamValid("id_name")),
    _use_exclude_id(isParamValid("exclude_id")),
    _use_interface_boundary_id_shift(isParamValid("interface_boundary_id_shift_pattern")),
    _boundary_quad_elem_type(
        getParam<MooseEnum>("boundary_region_element_type").template getEnum<QUAD_ELEM_TYPE>())
{
  declareMeshProperty("pattern_pitch_meta", 0.0);
  declareMeshProperty("input_pitch_meta", 0.0);
  declareMeshProperty<bool>("is_control_drum_meta", false);
  declareMeshProperty<std::vector<Point>>("control_drum_positions", std::vector<Point>());
  declareMeshProperty<std::vector<Real>>("control_drum_angles", std::vector<Real>());
  declareMeshProperty<std::vector<std::vector<Real>>>("control_drums_azimuthal_meta",
                                                      std::vector<std::vector<Real>>());
  declareMeshProperty<std::string>("position_file_name", getParam<std::string>("position_file"));
  declareMeshProperty<bool>("hexagon_peripheral_trimmability", !_generate_core_metadata);
  declareMeshProperty<bool>("hexagon_center_trimmability", true);
  declareMeshProperty<bool>("peripheral_modifier_compatible", _pattern_boundary == "hexagon");

  const unsigned int n_pattern_layers = _pattern.size();
  declareMeshProperty("pattern_size", n_pattern_layers);
  if (n_pattern_layers % 2 == 0)
    paramError(
        "pattern",
        "The length (layer number) of this parameter must be odd to ensure a hexagonal shape.");
  if (n_pattern_layers == 1)
    paramError("pattern", "The length (layer number) of this parameter must be larger than unity.");
  std::vector<unsigned int> pattern_max_array;
  std::vector<unsigned int> pattern_1d;
  for (unsigned int i = 0; i <= n_pattern_layers / 2; i++)
  {
    pattern_max_array.push_back(*std::max_element(_pattern[i].begin(), _pattern[i].end()));
    pattern_max_array.push_back(*std::max_element(_pattern[n_pattern_layers - 1 - i].begin(),
                                                  _pattern[n_pattern_layers - 1 - i].end()));
    pattern_1d.insert(pattern_1d.end(), _pattern[i].begin(), _pattern[i].end());
    pattern_1d.insert(pattern_1d.end(),
                      _pattern[n_pattern_layers - 1 - i].begin(),
                      _pattern[n_pattern_layers - 1 - i].end());
    if (_pattern[i].size() != n_pattern_layers / 2 + i + 1 ||
        _pattern[n_pattern_layers - 1 - i].size() != n_pattern_layers / 2 + i + 1)
      paramError(
          "pattern",
          "The two-dimentional array parameter pattern must have a correct hexagonal shape.");
  }
  if (*std::max_element(pattern_max_array.begin(), pattern_max_array.end()) >= _input_names.size())
    paramError("pattern",
               "Elements of this parameter must be smaller than the length of input_name.");
  if ((unsigned int)std::distance(pattern_1d.begin(),
                                  std::unique(pattern_1d.begin(), pattern_1d.end())) <
      _input_names.size())
    paramError("pattern", "All the meshes provided in inputs must be used here.");

  if (isParamValid("background_block_id"))
  {
    _peripheral_block_ids.push_back(getParam<subdomain_id_type>("background_block_id"));
    _peripheral_block_ids.insert(
        _peripheral_block_ids.end(), _duct_block_ids.begin(), _duct_block_ids.end());
  }
  else if (!_duct_block_ids.empty())
    paramError("background_block_id",
               "This parameter and duct_block_ids must be "
               "provided simultaneously.");
  if (isParamValid("background_block_name"))
  {
    _peripheral_block_names.push_back(getParam<SubdomainName>("background_block_name"));
    _peripheral_block_names.insert(
        _peripheral_block_names.end(), _duct_block_names.begin(), _duct_block_names.end());
  }
  else if (!_duct_block_names.empty())
    paramError("background_block_name",
               "This parameter and duct_block_names must be provided simultaneously.");

  if (_pattern_boundary == "hexagon")
  {
    for (unsigned int i = 1; i < _duct_sizes.size(); i++)
      if (_duct_sizes[i] <= _duct_sizes[i - 1])
        paramError("duct_sizes", "This parameter must be strictly ascending.");
    if (!_peripheral_block_ids.empty() && _peripheral_block_ids.size() != _duct_sizes.size() + 1)
      paramError("duct_block_ids",
                 "This parameter, if provided, must have a length equal to length of duct_sizes.");
    if (!_peripheral_block_names.empty() &&
        _peripheral_block_names.size() != _duct_sizes.size() + 1)
      paramError("duct_block_names",
                 "This parameter, if provided, must have a length equal to length of duct_sizes.");
  }
  else
  {
    if (!_peripheral_block_ids.empty() || !_peripheral_block_names.empty())
      paramError("background_block_id",
                 "This parameter and background_block_name must not be set when the "
                 "pattern_boundary is none.");
  }

  if (_use_interface_boundary_id_shift)
  {
    // check "interface_boundary_id_shift_pattern" parameter
    _interface_boundary_id_shift_pattern =
        getParam<std::vector<std::vector<boundary_id_type>>>("interface_boundary_id_shift_pattern");
    if (_interface_boundary_id_shift_pattern.size() != _pattern.size())
      paramError("interface_boundary_id_shift_pattern",
                 "This parameter, if provided, should have the same two-dimensional array shape as "
                 "the 'pattern' parameter. First dimension does not match");
    for (const auto i : make_range(_pattern.size()))
      if (_interface_boundary_id_shift_pattern[i].size() != _pattern[i].size())
        paramError("interface_boundary_id_shift_pattern",
                   "This parameter, if provided, should have the same two-dimensional array shape "
                   "as the 'pattern' parameter.");
  }
  // declare metadata for internal interface boundaries
  declareMeshProperty<bool>("interface_boundaries", false);
  declareMeshProperty<std::set<boundary_id_type>>("interface_boundary_ids", {});

  if (_use_reporting_id)
  {
    // get reporting id name input
    _reporting_id_names = getParam<std::vector<std::string>>("id_name");
    const unsigned int num_reporting_ids = _reporting_id_names.size();
    // get reporting id assign type input
    const auto input_assign_types = getParam<std::vector<MooseEnum>>("assign_type");
    if (input_assign_types.size() != num_reporting_ids)
      paramError("assign_type", "This parameter must have a length equal to length of id_name.");
    // list of reporting id names using manual id patterns;
    std::vector<std::string> manual_ids;
    for (const auto i : make_range(num_reporting_ids))
    {
      _assign_types.push_back(
          input_assign_types[i].getEnum<ReportingIDGeneratorUtils::AssignType>());
      if (_assign_types[i] == ReportingIDGeneratorUtils::AssignType::manual)
        manual_ids.push_back(_reporting_id_names[i]);
    }
    // processing "id_pattern" input parameter
    if (manual_ids.size() > 0 && !isParamValid("id_pattern"))
      paramError("id_pattern", "required when 'manual' is defined in \"assign_type\"");
    if (isParamValid("id_pattern"))
    {
      const auto input_id_patterns =
          getParam<std::vector<std::vector<std::vector<dof_id_type>>>>("id_pattern");
      if (input_id_patterns.size() != manual_ids.size())
        paramError("id_pattern",
                   "The number of patterns must be equal to the number of 'manual' types defined "
                   "in \"assign_type\".");
      for (unsigned int i = 0; i < manual_ids.size(); ++i)
        _id_patterns[manual_ids[i]] = input_id_patterns[i];
    }
    // processing exlude id
    _exclude_ids.resize(_input_names.size());
    if (_use_exclude_id)
    {
      std::vector<MeshGeneratorName> exclude_id_name =
          getParam<std::vector<MeshGeneratorName>>("exclude_id");
      for (unsigned int i = 0; i < _input_names.size(); ++i)
      {
        _exclude_ids[i] = false;
        for (auto input_name : exclude_id_name)
          if (_input_names[i] == input_name)
          {
            _exclude_ids[i] = true;
            break;
          }
      }
    }
    else
      for (unsigned int i = 0; i < _input_names.size(); ++i)
        _exclude_ids[i] = false;
  }
}

std::unique_ptr<MeshBase>
PatternedHexMeshGenerator::generate()
{
  std::vector<std::unique_ptr<ReplicatedMesh>> meshes(_input_names.size());
  for (const auto i : index_range(_input_names))
  {
    meshes[i] = dynamic_pointer_cast<ReplicatedMesh>(std::move(*_mesh_ptrs[i]));
    if (!meshes[i])
      paramError("inputs",
                 "Mesh '",
                 _input_names[i],
                 "' is not a replicated mesh. Only replicated meshes are supported");

    // throw an error message if the input mesh has a flat side up
    if (hasMeshProperty<bool>("flat_side_up", _input_names[i]))
      if (getMeshProperty<bool>("flat_side_up", _input_names[i]))
        paramError("inputs",
                   "Mesh '",
                   _input_names[i],
                   "' has a flat side facing up, which is not supported.");
  }

  std::vector<Real> pitch_array;
  std::vector<unsigned int> num_sectors_per_side_array;
  std::vector<unsigned int> num_sectors_per_side_array_tmp;
  std::vector<std::vector<Real>> control_drum_azimuthal_array;
  std::vector<unsigned int> background_intervals_array;
  std::vector<dof_id_type> node_id_background_array;
  std::vector<Real> max_radius_array;
  std::vector<bool> is_control_drum_array;
  Real max_radius_global(0.0);
  std::vector<Real> pattern_pitch_array;

  if (_pattern_boundary == "none" && _generate_core_metadata)
  {
    for (MooseIndex(_input_names) i = 0; i < _input_names.size(); ++i)
    {
      // throw an error message if the input mesh does not contain the required meta data
      if (!hasMeshProperty<Real>("pattern_pitch_meta", _input_names[i]))
        mooseError(
            "In PatternedHexMeshGenerator ",
            _name,
            ": the unit hexagonal input mesh does not contain appropriate meta data "
            "required for generating a core mesh. Involved input mesh: ",
            _input_names[i],
            "; metadata issue: 'pattern_pitch_meta' is missing. Note that "
            "'generate_core_metadata' is set to true, which"
            "means that the mesh generator is producing a core mesh by stitching the input "
            "assembly meshes together. Therefore,"
            "the input meshes must contain the metadata of assembly meshes, which can "
            "usually be either automatically assigned "
            "by using another PatternedHexMeshGenerator with 'generate_core_metadata' set as "
            "false and 'pattern_boundary' set as hexagon, or manually assigned by "
            "AddMetaDataGenerator.");
      pattern_pitch_array.push_back(getMeshProperty<Real>("pattern_pitch_meta", _input_names[i]));
      // throw an error message if the input mesh contains non-sense meta data
      if (pattern_pitch_array.back() == 0.0)
        mooseError(
            "In PatternedHexMeshGenerator ",
            _name,
            ": the unit hexagonal input mesh does not contain appropriate meta data "
            "required for generating a core mesh. Involved input mesh: ",
            _input_names[i],
            "; metadata issue: 'pattern_pitch_meta' is zero. Note that "
            "'generate_core_metadata' is set to true, which"
            "means that the mesh generator is producing a core mesh by stitching the input "
            "assembly meshes together. Therefore,"
            "the input meshes must contain the metadata of assembly meshes, which can "
            "usually be either automatically assigned "
            "by using another PatternedHexMeshGenerator with 'generate_core_metadata' set as "
            "false and 'pattern_boundary' set as hexagon, or manually assigned by "
            "AddMetaDataGenerator.");
      is_control_drum_array.push_back(
          getMeshProperty<bool>("is_control_drum_meta", _input_names[i]));
      control_drum_azimuthal_array.push_back(
          getMeshProperty<bool>("is_control_drum_meta", _input_names[i])
              ? getMeshProperty<std::vector<Real>>("azimuthal_angle_meta", _input_names[i])
              : std::vector<Real>());
    }
    if (!MooseUtils::absoluteFuzzyEqual(
            *std::max_element(pattern_pitch_array.begin(), pattern_pitch_array.end()),
            *std::min_element(pattern_pitch_array.begin(), pattern_pitch_array.end())))
      mooseError(
          "In PatternedHexMeshGenerator ",
          _name,
          ": pattern_pitch metadata values of all input mesh generators must be identical "
          "when pattern_boundary is 'none' and generate_core_metadata is true. Please check the "
          "parameters of the mesh generators that produce the input meshes."
          "Note that some of these mesh generators, such as "
          "HexagonConcentricCircleAdaptiveBoundaryMeshGenerator and FlexiblePatternGenerator,"
          "may have different definitions of hexagon size in their input parameters. Please refer "
          "to the documentation of these mesh generators.",
          pitchMetaDataErrorGenerator(_input_names, pattern_pitch_array, "pattern_pitch_meta"));
    else
    {
      _pattern_pitch = pattern_pitch_array.front();
      setMeshProperty("input_pitch_meta", _pattern_pitch);
    }
  }
  else
  {
    if (_pattern_boundary == "hexagon")
    {
      if (!isParamValid("hexagon_size"))
        paramError("hexagon_size",
                   "This parameter must be provided when pattern_boundary is hexagon.");
      else
        _pattern_pitch = 2.0 * (_hexagon_size_style == PolygonSizeStyle::apothem
                                    ? getParam<Real>("hexagon_size")
                                    : getParam<Real>("hexagon_size") * std::cos(M_PI / 6.0));
    }
    for (MooseIndex(_input_names) i = 0; i < _input_names.size(); ++i)
    {
      // throw an error message if the input mesh does not contain the required meta data
      if (!hasMeshProperty<Real>("pitch_meta", _input_names[i]))
        mooseError("In PatternedHexMeshGenerator ",
                   _name,
                   ": the unit hexagonal input mesh does not contain appropriate meta data "
                   "required for generating an assembly. Involved input mesh: ",
                   _input_names[i],
                   "; metadata issue: 'pitch_meta' is missing");
      pitch_array.push_back(getMeshProperty<Real>("pitch_meta", _input_names[i]));

      num_sectors_per_side_array_tmp =
          getMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta", _input_names[i]);
      if (*std::max_element(num_sectors_per_side_array_tmp.begin(),
                            num_sectors_per_side_array_tmp.end()) !=
          *std::min_element(num_sectors_per_side_array_tmp.begin(),
                            num_sectors_per_side_array_tmp.end()))
        mooseError("In PatternedHexMeshGenerator ",
                   _name,
                   ": num_sectors_per_side metadata values of all six sides of each input mesh "
                   "generator must be identical.");
      if (num_sectors_per_side_array_tmp.front() == 1 && _pattern_boundary == "hexagon")
        paramError(
            "inputs",
            "for each input mesh, the number of sectors on each side must be greater than unity.");
      num_sectors_per_side_array.push_back(*num_sectors_per_side_array_tmp.begin());
      background_intervals_array.push_back(
          getMeshProperty<unsigned int>("background_intervals_meta", _input_names[i]));
      node_id_background_array.push_back(
          getMeshProperty<dof_id_type>("node_id_background_meta", _input_names[i]));
      max_radius_array.push_back(getMeshProperty<Real>("max_radius_meta", _input_names[i]));
    }
    max_radius_global = *max_element(max_radius_array.begin(), max_radius_array.end());
    if (!MooseUtils::absoluteFuzzyEqual(*std::max_element(pitch_array.begin(), pitch_array.end()),
                                        *std::min_element(pitch_array.begin(), pitch_array.end())))
      mooseError("In PatternedHexMeshGenerator ",
                 _name,
                 ": pitch metadata values of all input mesh generators must be identical. Please "
                 "check the "
                 "parameters of the mesh generators that produce the input meshes.",
                 pitchMetaDataErrorGenerator(_input_names, pitch_array, "pitch_meta"));
    setMeshProperty("input_pitch_meta", pitch_array.front());
    if (*std::max_element(num_sectors_per_side_array.begin(), num_sectors_per_side_array.end()) !=
        *std::min_element(num_sectors_per_side_array.begin(), num_sectors_per_side_array.end()))
      mooseError(
          "In PatternedHexMeshGenerator ",
          _name,
          ": num_sectors_per_side metadata values of all input mesh generators must be identical.");
  }

  std::vector<Real> extra_dist;
  Real extra_dist_shift(0.0);
  Real y_min(0.0);
  Real y_max_0(0.0);
  Real y_max_n(0.0);
  const Real extra_dist_tol = _pattern_boundary == "hexagon" ? pitch_array.front() / 10.0 : 0.0;
  const Real extra_dist_shift_0 = _pattern_boundary == "hexagon" ? pitch_array.front() / 5.0 : 0.0;
  std::vector<unsigned int> peripheral_duct_intervals;
  if (_pattern_boundary == "hexagon")
  {
    if (_has_assembly_duct)
    {
      for (unsigned int i = 0; i < _duct_sizes.size(); i++)
      {
        if (_duct_sizes_style == PolygonSizeStyle::radius)
          _duct_sizes[i] /= std::cos(M_PI / 6.0);
        extra_dist.push_back(0.5 *
                             (_duct_sizes[i] * 2.0 - pitch_array.front() / std::sqrt(3.0) *
                                                         (Real)((_pattern.size() / 2) * 3 + 2)));
        peripheral_duct_intervals.push_back(_duct_intervals[i]);
      }
      if (_duct_sizes.back() >= _pattern_pitch / 2.0)
        paramError("duct_sizes",
                   "The duct sizes should not exceed the size of the hexagonal boundary.");
    }
    // calculate the distance between the larger hexagon boundary and the boundary of stitched unit
    // hexagons this is used to decide whether deformation is needed when cut-off happens or when
    // the distance is small.
    extra_dist.push_back(0.5 * (_pattern_pitch - pitch_array.front() / std::sqrt(3.0) *
                                                     (Real)((_pattern.size() / 2) * 3 + 2)));
    peripheral_duct_intervals.insert(peripheral_duct_intervals.begin(), _background_intervals);

    // In same cases, when the external hexagon size is small enough, the external hexagon boundary
    // may either be very close to the input hexagon meshes that are near the boundary or even cut
    // off the these hexagons. As long as the ring regions are not cut off, the input hexagons can
    // be deformed to accommodate the external hexagon shape. This block sets up the range of mesh
    // region that needs to be deformed.
    if (extra_dist.front() <= extra_dist_tol)
    {
      extra_dist_shift = extra_dist_shift_0 - extra_dist.front();
      for (Real & d : extra_dist)
        d += extra_dist_shift;
      y_min = _deform_non_circular_region
                  ? max_radius_global // Currently use this, ideally this should be the max of the
                                      // outer layer radii
                  : (pitch_array.front() / std::sqrt(3.0));
      y_max_0 = pitch_array.front() / std::sqrt(3.0) + extra_dist.front();
      y_max_n = y_max_0 - extra_dist_shift;
      if (y_max_n <= y_min)
        mooseError("In PatternedHexMeshGenerator ",
                   _name,
                   ": the assembly is cut off so much that the internal structure that should not "
                   "be altered is compromised.");
    }
  }

  setMeshProperty("pattern_pitch_meta", _pattern_pitch);

  // create a list of interface boundary ids for each input mesh
  // NOTE: list of interface boundary ids is stored in mesh metadata
  std::vector<std::set<boundary_id_type>> input_interface_boundary_ids;
  input_interface_boundary_ids.resize(_input_names.size());
  if (_use_interface_boundary_id_shift)
  {
    for (const auto i : make_range(_input_names.size()))
    {
      if (!hasMeshProperty<bool>("interface_boundaries", _input_names[i]))
        mooseError("Metadata 'interface_boundaries' could not be found on the input mesh: ",
                   _input_names[i]);
      if (!getMeshProperty<bool>("interface_boundaries", _input_names[i]))
        mooseError("Interface boundary ids were not constructed in the input mesh",
                   _input_names[i]);
      if (!hasMeshProperty<std::set<boundary_id_type>>("interface_boundary_ids", _input_names[i]))
        mooseError("Metadata 'interface_boundary_ids' could not be found on the input mesh: ",
                   _input_names[i]);
    }
  }
  for (const auto i : make_range(_input_names.size()))
    if (_use_interface_boundary_id_shift ||
        hasMeshProperty<std::set<boundary_id_type>>("interface_boundary_ids", _input_names[i]))
      input_interface_boundary_ids[i] =
          getMeshProperty<std::set<boundary_id_type>>("interface_boundary_ids", _input_names[i]);

  int x_mov = 0;

  const Real input_pitch((_pattern_boundary == "hexagon" || !_generate_core_metadata)
                             ? pitch_array.front()
                             : _pattern_pitch);
  std::vector<Real> control_drum_positions_x;
  std::vector<Real> control_drum_positions_y;
  std::vector<std::vector<Real>> control_drum_azimuthals;

  std::unique_ptr<ReplicatedMesh> out_mesh;

  for (unsigned i = 0; i < _pattern.size(); i++)
  {
    Real deltax = -x_mov * input_pitch / 2;
    if (i == _pattern.size() - 1)
      x_mov--;
    else if (_pattern[i].size() < _pattern[i + 1].size())
      x_mov++;
    else
      x_mov--;
    Real deltay = -(Real)(i)*input_pitch * std::sin(M_PI / 3);

    for (unsigned int j = 0; j < _pattern[i].size(); j++)
    {
      const auto pattern = _pattern[i][j];
      ReplicatedMesh & pattern_mesh = *meshes[pattern];

      // No pattern boundary, so no need to wrap with a peripheral mesh
      // Use the inputs as they stand and translate accordingly later
      if (_pattern_boundary == "none")
      {
        if (_generate_core_metadata && is_control_drum_array[pattern] == 1)
        {
          control_drum_positions_x.push_back(deltax + j * input_pitch);
          control_drum_positions_y.push_back(deltay);
          control_drum_azimuthals.push_back(control_drum_azimuthal_array[pattern]);
        }

        if (j == 0 && i == 0)
        {
          out_mesh = dynamic_pointer_cast<ReplicatedMesh>(pattern_mesh.clone());
          if (_assign_control_drum_id && _generate_core_metadata)
            out_mesh->add_elem_integer(
                "control_drum_id",
                true,
                is_control_drum_array[pattern] ? control_drum_azimuthals.size() : 0);
          // Reassign interface boundary ids
          if (_use_interface_boundary_id_shift)
            reassignBoundaryIDs(*out_mesh,
                                _interface_boundary_id_shift_pattern[i][j],
                                input_interface_boundary_ids[pattern]);
          continue;
        }
      }
      // Has a pattern boundary so we need to wrap with a peripheral mesh
      else // has a pattern boundary
      {
        Real rotation_angle = std::numeric_limits<Real>::max();
        Real orientation = std::numeric_limits<Real>::max();
        unsigned int mesh_type = std::numeric_limits<unsigned int>::max();
        bool on_periphery = true;

        if (j == 0 && i == 0)
        {
          rotation_angle = 60.;
          mesh_type = CORNER_MESH;
          orientation = 0.;
        }
        else if (j == 0 && i == _pattern.size() - 1)
        {
          rotation_angle = 180.;
          mesh_type = CORNER_MESH;
          orientation = -120.;
        }
        else if (j == 0 && i == (_pattern.size() - 1) / 2)
        {
          rotation_angle = 120.;
          mesh_type = CORNER_MESH;
          orientation = -60.;
        }
        else if (j == _pattern[i].size() - 1 && i == 0)
        {
          rotation_angle = 0.;
          mesh_type = CORNER_MESH;
          orientation = 60.;
        }
        else if (j == _pattern[i].size() - 1 && i == _pattern.size() - 1)
        {
          rotation_angle = -120.;
          mesh_type = CORNER_MESH;
          orientation = 180.;
        }
        else if (j == _pattern[i].size() - 1 && i == (_pattern.size() - 1) / 2)
        {
          rotation_angle = -60.;
          mesh_type = CORNER_MESH;
          orientation = 120.;
        }
        else if (i == 0)
        {
          rotation_angle = 0.;
          mesh_type = BOUNDARY_MESH;
          orientation = 0.;
        }
        else if (i == _pattern.size() - 1)
        {
          rotation_angle = 180.;
          mesh_type = BOUNDARY_MESH;
          orientation = -180.;
        }
        else if (j == 0 && i < (_pattern.size() - 1) / 2)
        {
          rotation_angle = 60.;
          mesh_type = BOUNDARY_MESH;
          orientation = -60.;
        }
        else if (j == 0 && i > (_pattern.size() - 1) / 2)
        {
          rotation_angle = 120.;
          mesh_type = BOUNDARY_MESH;
          orientation = -120.;
        }
        else if (j == _pattern[i].size() - 1 && i < (_pattern.size() - 1) / 2)
        {
          rotation_angle = -60.;
          mesh_type = BOUNDARY_MESH;
          orientation = 60.;
        }
        else if (j == _pattern[i].size() - 1 && i > (_pattern.size() - 1) / 2)
        {
          rotation_angle = -120.;
          mesh_type = BOUNDARY_MESH;
          orientation = 120;
        }
        else
          on_periphery = false;

        if (on_periphery)
        {
          auto tmp_peripheral_mesh = dynamic_pointer_cast<ReplicatedMesh>(pattern_mesh.clone());
          addPeripheralMesh(*tmp_peripheral_mesh,
                            pattern,
                            pitch_array.front(),
                            extra_dist,
                            num_sectors_per_side_array,
                            peripheral_duct_intervals,
                            rotation_angle,
                            mesh_type);

          if (extra_dist_shift != 0)
            cutOffPolyDeform(*tmp_peripheral_mesh, orientation, y_max_0, y_max_n, y_min, mesh_type);

          // Reassign interface boundary ids
          if (_use_interface_boundary_id_shift)
            reassignBoundaryIDs(*tmp_peripheral_mesh,
                                _interface_boundary_id_shift_pattern[i][j],
                                input_interface_boundary_ids[pattern]);

          if (i == 0 && j == 0)
            out_mesh = std::move(tmp_peripheral_mesh);
          else
          {
            // Retrieve subdomain name map from the mesh to be stitched and insert it to the main
            // subdomain map
            const auto & increment_subdomain_map = tmp_peripheral_mesh->get_subdomain_name_map();
            out_mesh->set_subdomain_name_map().insert(increment_subdomain_map.begin(),
                                                      increment_subdomain_map.end());

            MeshTools::Modification::translate(
                *tmp_peripheral_mesh, deltax + j * input_pitch, deltay, 0);
            out_mesh->stitch_meshes(*tmp_peripheral_mesh,
                                    OUTER_SIDESET_ID,
                                    OUTER_SIDESET_ID,
                                    TOLERANCE,
                                    /*clear_stitched_boundary_ids=*/true,
                                    /*verbose=*/false);
          }

          continue;
        }
      }

      if (_assign_control_drum_id && _pattern_boundary == "none" && _generate_core_metadata)
        pattern_mesh.add_elem_integer(
            "control_drum_id",
            true,
            is_control_drum_array[pattern] ? control_drum_azimuthals.size() : 0);

      // Translate to correct position
      MeshTools::Modification::translate(pattern_mesh, deltax + j * input_pitch, deltay, 0);

      // Define a reference map variable for subdomain map
      auto & main_subdomain_map = out_mesh->set_subdomain_name_map();
      // Retrieve subdomain name map from the mesh to be stitched and insert it to the main
      // subdomain map
      const auto & increment_subdomain_map = pattern_mesh.get_subdomain_name_map();
      main_subdomain_map.insert(increment_subdomain_map.begin(), increment_subdomain_map.end());
      // Check if one SubdomainName is shared by more than one subdomain ids
      std::set<SubdomainName> main_subdomain_map_name_list;
      for (auto const & id_name_pair : main_subdomain_map)
        main_subdomain_map_name_list.emplace(id_name_pair.second);
      if (main_subdomain_map.size() != main_subdomain_map_name_list.size())
        paramError("inputs", "The input meshes contain subdomain name maps with conflicts.");
      // Shift interface boundary ids
      if (_use_interface_boundary_id_shift)
        reassignBoundaryIDs(pattern_mesh,
                            _interface_boundary_id_shift_pattern[i][j],
                            input_interface_boundary_ids[pattern]);

      out_mesh->stitch_meshes(pattern_mesh,
                              OUTER_SIDESET_ID,
                              OUTER_SIDESET_ID,
                              TOLERANCE,
                              /*clear_stitched_boundary_ids=*/false,
                              /*verbose=*/false);

      // Translate back now that we've stitched so that anyone else that uses this mesh has it at
      // the origin
      MeshTools::Modification::translate(pattern_mesh, -(deltax + j * input_pitch), -deltay, 0);
      // Roll back the changes in interface boundary ids for the same reason
      if (_use_interface_boundary_id_shift)
        reassignBoundaryIDs(pattern_mesh,
                            _interface_boundary_id_shift_pattern[i][j],
                            input_interface_boundary_ids[pattern],
                            true);
    }
  }

  // Check if OUTER_SIDESET_ID is really the external boundary. Correct if needed.
  auto side_list = out_mesh->get_boundary_info().build_side_list();
  for (auto & sl : side_list)
  {
    if (std::get<2>(sl) == OUTER_SIDESET_ID)
      if (out_mesh->elem_ptr(std::get<0>(sl))->neighbor_ptr(std::get<1>(sl)) != nullptr)
        out_mesh->get_boundary_info().remove_side(
            out_mesh->elem_ptr(std::get<0>(sl)), std::get<1>(sl), std::get<2>(sl));
  }
  out_mesh->get_boundary_info().clear_boundary_node_ids();

  out_mesh->get_boundary_info().build_node_list_from_side_list();
  const auto node_list = out_mesh->get_boundary_info().build_node_list();

  std::vector<Real> bd_x_list;
  std::vector<Real> bd_y_list;
  std::vector<std::pair<Real, dof_id_type>> node_azi_list;
  const Point origin_pt = MooseMeshUtils::meshCentroidCalculator(*out_mesh);
  const Real origin_x = origin_pt(0);
  const Real origin_y = origin_pt(1);

  MeshTools::Modification::translate(*out_mesh, -origin_x, -origin_y, 0);

  if (_uniform_mesh_on_sides)
  {
    const Real azi_tol = 1E-8;
    for (unsigned int i = 0; i < node_list.size(); ++i)
    {
      if (std::get<1>(node_list[i]) == OUTER_SIDESET_ID)
      {
        node_azi_list.push_back(
            std::make_pair(atan2(out_mesh->node_ref(std::get<0>(node_list[i]))(1),
                                 out_mesh->node_ref(std::get<0>(node_list[i]))(0)) *
                               180.0 / M_PI,
                           std::get<0>(node_list[i])));
        // correct the last node's angle value (180.0) if it becomes a negative value.
        if (node_azi_list.back().first + 180.0 <= azi_tol)
          node_azi_list.back().first = 180;
      }
    }
    std::sort(node_azi_list.begin(), node_azi_list.end());
    const unsigned int side_intervals = node_azi_list.size() / HEXAGON_NUM_SIDES;
    for (unsigned int i = 0; i < HEXAGON_NUM_SIDES; i++)
    {
      for (unsigned int j = 1; j <= side_intervals; j++)
      {
        Real azi_corr_tmp = atan2((Real)j * 2.0 / (Real)side_intervals - 1.0, std::sqrt(3.0));
        Real x_tmp = _pattern_pitch / 2.0;
        Real y_tmp = x_tmp * std::tan(azi_corr_tmp);
        nodeCoordRotate(x_tmp, y_tmp, (Real)i * 60.0 - 150.0);
        Point p_tmp = Point(x_tmp, y_tmp, 0.0);
        out_mesh->add_point(p_tmp, node_azi_list[i * side_intervals + j - 1].second);
      }
    }

    // if quadratic elements are used, additional nodes need to be adjusted based on the new
    // boundary node locations. adjust side mid-edge nodes to the midpoints of the corner
    // points, and if QUAD9, adjust center point to new centroid.
    if (_boundary_quad_elem_type != QUAD_ELEM_TYPE::QUAD4)
      adjustPeripheralQuadraticElements(*out_mesh, _boundary_quad_elem_type);
  }

  MeshTools::Modification::rotate(*out_mesh, _rotate_angle, 0.0, 0.0);

  // This combination of input parameters is usually used for core mesh generation by stitching
  // assembly meshes together.
  if (_pattern_boundary == "none" && _generate_core_metadata)
  {
    const Real azi_tol = 1E-8;
    std::vector<std::tuple<Real, Point, std::vector<Real>, dof_id_type>> control_drum_tmp;
    std::vector<dof_id_type> control_drum_id_sorted;
    unsigned int id = out_mesh->get_elem_integer_index("control_drum_id");
    for (unsigned int i = 0; i < control_drum_positions_x.size(); ++i)
    {
      control_drum_positions_x[i] -= origin_x;
      control_drum_positions_y[i] -= origin_y;
      nodeCoordRotate(control_drum_positions_x[i], control_drum_positions_y[i], _rotate_angle);
      Real cd_angle = atan2(control_drum_positions_y[i], control_drum_positions_x[i]);

      for (unsigned int j = 0; j < control_drum_azimuthals[i].size(); j++)
      {
        control_drum_azimuthals[i][j] += _rotate_angle;
        control_drum_azimuthals[i][j] =
            atan2(std::sin(control_drum_azimuthals[i][j] / 180.0 * M_PI),
                  std::cos(control_drum_azimuthals[i][j] / 180.0 * M_PI)) /
            M_PI * 180.0; // quick way to move to -M_PI to M_PI
      }
      std::sort(control_drum_azimuthals[i].begin(), control_drum_azimuthals[i].end());

      if (std::abs(cd_angle) < azi_tol)
        cd_angle = 0;
      else if (cd_angle < 0.0)
        cd_angle += 2 * M_PI;
      control_drum_tmp.push_back(
          std::make_tuple(cd_angle,
                          Point(control_drum_positions_x[i], control_drum_positions_y[i], 0.0),
                          control_drum_azimuthals[i],
                          i + 1)); // control drum index to help sort control_drum_id
    }
    std::sort(control_drum_tmp.begin(), control_drum_tmp.end());
    std::vector<Point> control_drum_positions;
    std::vector<Real> control_drum_angles;
    std::vector<std::vector<Real>> control_drums_azimuthal_meta;
    for (unsigned int i = 0; i < control_drum_tmp.size(); ++i)
    {
      control_drum_positions.push_back(std::get<1>(control_drum_tmp[i]));
      control_drum_angles.push_back(std::get<0>(control_drum_tmp[i]));
      control_drums_azimuthal_meta.push_back(std::get<2>(control_drum_tmp[i]));
      control_drum_id_sorted.push_back(std::get<3>(control_drum_tmp[i]));
    }
    setMeshProperty("control_drum_positions", control_drum_positions);
    setMeshProperty("control_drum_angles", control_drum_angles);
    setMeshProperty("control_drums_azimuthal_meta", control_drums_azimuthal_meta);

    if (_assign_control_drum_id)
    {
      for (const auto & elem : out_mesh->element_ptr_range())
      {
        dof_id_type unsorted_control_drum_id = elem->get_extra_integer(id);
        if (unsorted_control_drum_id != 0)
        {
          auto sorted_iter = std::find(control_drum_id_sorted.begin(),
                                       control_drum_id_sorted.end(),
                                       unsorted_control_drum_id);
          elem->set_extra_integer(id,
                                  std::distance(control_drum_id_sorted.begin(), sorted_iter) + 1);
        }
      }
    }

    if (_generate_control_drum_positions_file)
    {
      std::string position_file_name = getMeshProperty<std::string>("position_file_name", name());
      std::ofstream pos_file(position_file_name);
      for (unsigned int i = 0; i < control_drum_positions.size(); ++i)
        pos_file << control_drum_positions[i](0) << " " << control_drum_positions[i](1) << " 0.0\n";
      pos_file.close();
    }
  }

  // before return, add reporting IDs if _use_reporting_id is set true
  if (_use_reporting_id)
    addReportingIDs(*out_mesh, meshes);

  // Assign customized peripheral block ids and names
  if (!_peripheral_block_ids.empty())
    for (const auto & elem : out_mesh->active_element_ptr_range())
      for (subdomain_id_type i = PERIPHERAL_ID_SHIFT; i <= PERIPHERAL_ID_SHIFT + _duct_sizes.size();
           i++)
        if (elem->subdomain_id() == i)
        {
          elem->subdomain_id() = _peripheral_block_ids[i - PERIPHERAL_ID_SHIFT];
          break;
        }
  if (!_peripheral_block_names.empty())
  {
    for (unsigned i = 0; i < _peripheral_block_names.size(); i++)
      out_mesh->subdomain_name(_peripheral_block_ids.empty() ? (PERIPHERAL_ID_SHIFT + i)
                                                             : _peripheral_block_ids[i]) =
          _peripheral_block_names[i];
  }
  // Assign customized outer surface boundary id and name
  if (_external_boundary_id > 0)
    MooseMesh::changeBoundaryId(*out_mesh, OUTER_SIDESET_ID, _external_boundary_id, false);
  if (!_external_boundary_name.empty())
  {
    out_mesh->get_boundary_info().sideset_name(
        _external_boundary_id > 0 ? _external_boundary_id : (boundary_id_type)OUTER_SIDESET_ID) =
        _external_boundary_name;
    out_mesh->get_boundary_info().nodeset_name(
        _external_boundary_id > 0 ? _external_boundary_id : (boundary_id_type)OUTER_SIDESET_ID) =
        _external_boundary_name;
  }
  // Merge the boundary name maps of all the input meshed to generate the output mesh's boundary
  // name maps
  auto & new_sideset_map = out_mesh->get_boundary_info().set_sideset_name_map();
  auto & new_nodeset_map = out_mesh->get_boundary_info().set_nodeset_name_map();
  for (unsigned int i = 0; i < meshes.size(); i++)
  {
    const auto input_sideset_map = meshes[i]->get_boundary_info().get_sideset_name_map();
    new_sideset_map.insert(input_sideset_map.begin(), input_sideset_map.end());
    const auto input_nodeset_map = meshes[i]->get_boundary_info().get_nodeset_name_map();
    new_nodeset_map.insert(input_nodeset_map.begin(), input_nodeset_map.end());
  }

  // set mesh metadata related with interface boundary ids
  const std::set<boundary_id_type> boundary_ids = out_mesh->get_boundary_info().get_boundary_ids();
  const std::set<boundary_id_type> interface_boundary_ids = getInterfaceBoundaryIDs(
      _pattern,
      _interface_boundary_id_shift_pattern,
      boundary_ids,
      input_interface_boundary_ids,
      _use_interface_boundary_id_shift,
      _create_inward_interface_boundaries || _create_outward_interface_boundaries,
      extra_dist.size());
  if (interface_boundary_ids.size() > 0)
  {
    setMeshProperty("interface_boundaries", true);
    setMeshProperty("interface_boundary_ids", interface_boundary_ids);
  }

  out_mesh->set_isnt_prepared();
  auto mesh = dynamic_pointer_cast<MeshBase>(out_mesh);
  return mesh;
}

void
PatternedHexMeshGenerator::addPeripheralMesh(
    ReplicatedMesh & mesh,
    const unsigned int pattern,
    const Real pitch,
    const std::vector<Real> & extra_dist,
    const std::vector<unsigned int> & num_sectors_per_side_array,
    const std::vector<unsigned int> & peripheral_duct_intervals,
    const Real rotation_angle,
    const unsigned int mesh_type)
{
  std::vector<std::pair<Real, Real>> positions_inner;
  std::vector<std::pair<Real, Real>> d_positions_outer;

  std::vector<std::vector<unsigned int>> peripheral_point_index;
  std::vector<std::pair<Real, Real>> sub_positions_inner;
  std::vector<std::pair<Real, Real>> sub_d_positions_outer;

  if (mesh_type == CORNER_MESH)
    // corner mesh has three sides that need peripheral meshes.
    // each element has three sub-elements, representing beginning, middle, and ending azimuthal
    // points
    peripheral_point_index = {{0, 1, 2}, {2, 3, 4}, {4, 5, 6}};
  else
    // side mesh has two sides that need peripheral meshes.
    peripheral_point_index = {{0, 1, 2}, {2, 7, 8}};

  // extra_dist includes background and ducts.
  // Loop to calculate the positions of the boundaries.
  for (unsigned int i = 0; i < extra_dist.size(); i++)
  {
    positionSetup(positions_inner,
                  d_positions_outer,
                  i == 0 ? 0.0 : extra_dist[i - 1],
                  extra_dist[i],
                  pitch,
                  i);

    // Loop for all applicable sides that need peripheral mesh (3 for corner and 2 for edge)
    for (unsigned int peripheral_index = 0; peripheral_index < peripheral_point_index.size();
         peripheral_index++)
    {
      // Loop for beginning, middle and ending positions of a side
      for (unsigned int vector_index = 0; vector_index < 3; vector_index++)
      {
        sub_positions_inner.push_back(
            positions_inner[peripheral_point_index[peripheral_index][vector_index]]);
        sub_d_positions_outer.push_back(
            d_positions_outer[peripheral_point_index[peripheral_index][vector_index]]);
      }
      auto meshp0 = buildSimplePeripheral(num_sectors_per_side_array[pattern],
                                          peripheral_duct_intervals[i],
                                          sub_positions_inner,
                                          sub_d_positions_outer,
                                          i,
                                          _boundary_quad_elem_type,
                                          _create_inward_interface_boundaries,
                                          (i != extra_dist.size() - 1) &&
                                              _create_outward_interface_boundaries);

      if (mesh.is_prepared()) // Need to prepare if the other is prepared to stitch
        meshp0->prepare_for_use();

      // rotate the peripheral mesh to the desired side of the hexagon.
      MeshTools::Modification::rotate(*meshp0, rotation_angle, 0, 0);
      mesh.stitch_meshes(*meshp0, OUTER_SIDESET_ID, OUTER_SIDESET_ID, TOLERANCE, true, false);
      sub_positions_inner.resize(0);
      sub_d_positions_outer.resize(0);
    }
  }
}

void
PatternedHexMeshGenerator::positionSetup(std::vector<std::pair<Real, Real>> & positions_inner,
                                         std::vector<std::pair<Real, Real>> & d_positions_outer,
                                         const Real extra_dist_in,
                                         const Real extra_dist_out,
                                         const Real pitch,
                                         const unsigned int radial_index) const
{
  positions_inner.resize(0);
  d_positions_outer.resize(0);

  // Nine sets of positions are generated here, as shown below.
  // CORNER MESH Peripheral {0 1 2}, {2 3 4} and {4 5 6}
  //           3       2   1   0
  //            \      :   :   :
  //             \     :   :   :
  //      4.       .       :   :
  //         ` .               :
  //      5.   |               |
  //         ` |               |
  //      6.   |               |
  //         ` |               |
  //               .       .
  //                   .
  //
  // EDGE MESH Peripheral {0 1 2} and {2 7 8}
  //           8   7   2   1   0
  //           :   :   :   :   :
  //           :   :   :   :   :
  //           :   :       :   :
  //           :               :
  //           |               |
  //           |               |
  //           |               |
  //           |               |
  //               .       .
  //                   .

  positions_inner.push_back(
      std::make_pair(-pitch / 2.0,
                     std::sqrt(3.0) * pitch / 6.0 +
                         (radial_index != 0 ? std::sqrt(3.0) * pitch / 6.0 + extra_dist_in : 0.0)));
  positions_inner.push_back(std::make_pair(
      -pitch / 4.0,
      std::sqrt(3.0) * pitch / 4.0 +
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 12.0 + extra_dist_in : 0.0)));
  positions_inner.push_back(std::make_pair(
      0.0, std::sqrt(3.0) * pitch / 3.0 + (radial_index != 0 ? extra_dist_in : 0.0)));
  positions_inner.push_back(std::make_pair(
      pitch / 4.0 + (radial_index != 0 ? pitch / 12.0 + std::sqrt(3.0) * extra_dist_in / 3.0 : 0.0),
      std::sqrt(3.0) * pitch / 4.0 +
          (radial_index != 0 ? pitch * std::sqrt(3.0) / 12.0 + extra_dist_in : 0.0)));
  positions_inner.push_back(std::make_pair(
      pitch / 2.0 + (radial_index != 0 ? std::sqrt(3.0) * extra_dist_in / 2.0 : 0.0),
      std::sqrt(3.0) * pitch / 6.0 + (radial_index != 0 ? extra_dist_in / 2.0 : 0.0)));
  positions_inner.push_back(std::make_pair(
      pitch / 2.0 + (radial_index != 0 ? pitch / 8.0 + std::sqrt(3.0) * extra_dist_in / 2.0 : 0.0),
      0.0 + (radial_index != 0 ? std::sqrt(3.0) * pitch / 24.0 + extra_dist_in / 2.0 : 0.0)));
  positions_inner.push_back(std::make_pair(
      pitch / 2.0 + (radial_index != 0 ? pitch / 4.0 + std::sqrt(3.0) * extra_dist_in / 2.0 : 0.0),
      -std::sqrt(3.0) * pitch / 6.0 +
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 12.0 + extra_dist_in / 2.0 : 0.0)));
  positions_inner.push_back(std::make_pair(
      pitch / 4.0,
      std::sqrt(3.0) * pitch / 4.0 +
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 12.0 + extra_dist_in : 0.0)));
  positions_inner.push_back(
      std::make_pair(pitch / 2.0,
                     std::sqrt(3.0) * pitch / 6.0 +
                         (radial_index != 0 ? std::sqrt(3.0) * pitch / 6.0 + extra_dist_in : 0.0)));

  d_positions_outer.push_back(
      std::make_pair(0.0,
                     std::sqrt(3.0) * pitch / 6.0 + extra_dist_out -
                         (radial_index != 0 ? std::sqrt(3.0) * pitch / 6.0 + extra_dist_in : 0.0)));
  d_positions_outer.push_back(std::make_pair(
      0.0,
      std::sqrt(3.0) * pitch / 12.0 + extra_dist_out -
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 12.0 + extra_dist_in : 0.0)));
  d_positions_outer.push_back(
      std::make_pair(0.0, extra_dist_out - (radial_index != 0 ? extra_dist_in : 0.0)));
  d_positions_outer.push_back(std::make_pair(
      pitch / 12.0 + std::sqrt(3.0) * extra_dist_out / 3.0 -
          (radial_index != 0 ? pitch / 12.0 + std::sqrt(3.0) * extra_dist_in / 3.0 : 0.0),
      pitch * std::sqrt(3.0) / 12.0 + extra_dist_out -
          (radial_index != 0 ? pitch * std::sqrt(3.0) / 12.0 + extra_dist_in : 0.0)));
  d_positions_outer.push_back(
      std::make_pair(std::sqrt(3.0) * extra_dist_out / 2.0 -
                         (radial_index != 0 ? std::sqrt(3.0) * extra_dist_in / 2.0 : 0.0),
                     extra_dist_out / 2.0 - (radial_index != 0 ? extra_dist_in / 2.0 : 0.0)));
  d_positions_outer.push_back(std::make_pair(
      pitch / 8.0 + std::sqrt(3.0) * extra_dist_out / 2.0 -
          (radial_index != 0 ? pitch / 8.0 + std::sqrt(3.0) * extra_dist_in / 2.0 : 0.0),
      std::sqrt(3.0) * pitch / 24.0 + extra_dist_out / 2.0 -
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 24.0 + extra_dist_in / 2.0 : 0.0)));
  d_positions_outer.push_back(std::make_pair(
      pitch / 4.0 + std::sqrt(3.0) * extra_dist_out / 2.0 -
          (radial_index != 0 ? pitch / 4.0 + std::sqrt(3.0) * extra_dist_in / 2.0 : 0.0),
      std::sqrt(3.0) * pitch / 12.0 + extra_dist_out / 2.0 -
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 12.0 + extra_dist_in / 2.0 : 0.0)));
  d_positions_outer.push_back(std::make_pair(
      0.0,
      std::sqrt(3.0) * pitch / 12.0 + extra_dist_out -
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 12.0 + extra_dist_in : 0.0)));
  d_positions_outer.push_back(
      std::make_pair(0.0,
                     std::sqrt(3.0) * pitch / 6.0 + extra_dist_out -
                         (radial_index != 0 ? std::sqrt(3.0) * pitch / 6.0 + extra_dist_in : 0.0)));
}

void
PatternedHexMeshGenerator::addReportingIDs(
    MeshBase & mesh, const std::vector<std::unique_ptr<ReplicatedMesh>> & from_meshes) const
{
  const unsigned int num_reporting_ids = _reporting_id_names.size();
  for (unsigned int i = 0; i < num_reporting_ids; ++i)
  {
    const std::string element_id_name = _reporting_id_names[i];
    unsigned int extra_id_index;
    if (!mesh.has_elem_integer(element_id_name))
      extra_id_index = mesh.add_elem_integer(element_id_name);
    else
    {
      extra_id_index = mesh.get_elem_integer_index(element_id_name);
      paramWarning(
          "id_name", "An element integer with the name '", element_id_name, "' already exists");
    }

    // assign reporting IDs to individual elements
    // NOTE: background block id should be set "PERIPHERAL_ID_SHIFT" because this function is called
    // before assigning the user-defined background block id
    std::set<subdomain_id_type> background_block_ids =
        (isParamValid("background_block_id")) ? std::set<subdomain_id_type>({PERIPHERAL_ID_SHIFT})
                                              : std::set<subdomain_id_type>();

    const bool using_manual_id =
        (_assign_types[i] == ReportingIDGeneratorUtils::AssignType::manual);
    ReportingIDGeneratorUtils::assignReportingIDs(mesh,
                                                  extra_id_index,
                                                  _assign_types[i],
                                                  _use_exclude_id,
                                                  _exclude_ids,
                                                  _pattern_boundary == "hexagon",
                                                  background_block_ids,
                                                  from_meshes,
                                                  _pattern,
                                                  (using_manual_id)
                                                      ? _id_patterns.at(element_id_name)
                                                      : std::vector<std::vector<dof_id_type>>());
  }
}
