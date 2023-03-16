//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PatternedCartesianMeshGenerator.h"
#include "ReportingIDGeneratorUtils.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)
#include <fstream> // used to generate the optional control drum position file

registerMooseObject("ReactorApp", PatternedCartesianMeshGenerator);

InputParameters
PatternedCartesianMeshGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  params.addRequiredParam<std::vector<MeshGeneratorName>>(
      "inputs", "The names of the meshes forming the pattern.");
  params.addRequiredRangeCheckedParam<std::vector<std::vector<unsigned int>>>(
      "pattern",
      "pattern>=0",
      "A two-dimensional cartesian (square-shaped) array starting with the upper-left corner."
      "It is composed of indexes into the inputs vector");
  MooseEnum cartesian_pattern_boundary("none expanded", "expanded");
  params.addParam<MooseEnum>(
      "pattern_boundary", cartesian_pattern_boundary, "The boundary shape of the patterned mesh.");
  params.addParam<bool>(
      "generate_core_metadata",
      false,
      "A Boolean parameter that controls whether the core related metadata "
      "is generated for other MOOSE objects such as 'MultiControlDrumFunction' or not.");
  params.addRangeCheckedParam<unsigned int>("background_intervals",
                                            3,
                                            "background_intervals>0",
                                            "Radial intervals in the assembly peripheral region.");
  params.addRangeCheckedParam<Real>(
      "square_size",
      "square_size>0.0",
      "Size (side length) of the outmost square boundary to be generated; this is "
      "required only when pattern type is 'expanded'.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "duct_sizes", "duct_sizes>0.0", "Distance(s) from center to duct(s) inner boundaries.");
  MooseEnum duct_sizes_style("apothem radius", "apothem");
  params.addParam<MooseEnum>("duct_sizes_style",
                             duct_sizes_style,
                             "Style in which square center to duct distance(s) is given (apothem "
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
  // A square pattern_boundary mesh can be used in "inputs" of `PatternedCartesianMeshGenerator`
  // without rotation or with rotation of 90, 180, or 270 degrees.
  params.addParam<Real>(
      "rotate_angle",
      0.0,
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
  params.addParam<std::string>(
      "external_boundary_name", std::string(), "Optional customized external boundary name.");
  params.addParam<bool>("deform_non_circular_region",
                        true,
                        "Whether the non-circular region (outside the rings) can be deformed.");
  params.addParam<std::string>("id_name", "Name of extra integer ID set");
  params.addParam<std::vector<MeshGeneratorName>>(
      "exclude_id", "Name of input meshes to be excluded in ID generation");
  MooseEnum option("cell pattern manual", "cell");
  params.addParam<MooseEnum>("assign_type", option, "Type of integer ID assignment");
  params.addParam<std::vector<std::vector<dof_id_type>>>(
      "id_pattern",
      "User-defined element IDs. A double-indexed array starting with the upper-left corner");
  params.addParamNamesToGroup(
      "pattern_boundary background_block_id background_block_name duct_block_ids duct_block_names "
      "external_boundary_id external_boundary_name create_inward_interface_boundaries "
      "create_outward_interface_boundaries",
      "Customized Subdomain/Boundary");
  params.addParamNamesToGroup(
      "generate_control_drum_positions_file assign_control_drum_id position_file", "Control Drum");
  params.addParamNamesToGroup(
      "background_intervals duct_intervals uniform_mesh_on_sides deform_non_circular_region",
      "Mesh Density");
  params.addParamNamesToGroup("id_name exclude_id assign_type id_pattern", "Reporting ID");
  params.addClassDescription(
      "This PatternedCartesianMeshGenerator source code assembles square meshes into a square "
      "grid "
      "and optionally forces the outer boundary to be square and/or adds a duct.");

  return params;
}

PatternedCartesianMeshGenerator::PatternedCartesianMeshGenerator(const InputParameters & parameters)
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
    _external_boundary_name(getParam<std::string>("external_boundary_name")),
    _create_inward_interface_boundaries(getParam<bool>("create_inward_interface_boundaries")),
    _create_outward_interface_boundaries(getParam<bool>("create_outward_interface_boundaries")),
    _deform_non_circular_region(getParam<bool>("deform_non_circular_region")),
    _use_reporting_id(isParamValid("id_name")),
    _assign_type(
        getParam<MooseEnum>("assign_type").getEnum<ReportingIDGeneratorUtils::AssignType>()),
    _use_exclude_id(isParamValid("exclude_id"))
{
  declareMeshProperty("pattern_pitch_meta", 0.0);
  declareMeshProperty("input_pitch_meta", 0.0);
  declareMeshProperty<bool>("is_control_drum_meta", false);
  declareMeshProperty<std::vector<Point>>("control_drum_positions", std::vector<Point>());
  declareMeshProperty<std::vector<Real>>("control_drum_angles", std::vector<Real>());
  declareMeshProperty<std::vector<std::vector<Real>>>("control_drums_azimuthal_meta",
                                                      std::vector<std::vector<Real>>());
  declareMeshProperty<std::string>("position_file_name", getParam<std::string>("position_file"));
  declareMeshProperty<bool>("square_peripheral_trimmability", !_generate_core_metadata);
  declareMeshProperty<bool>("square_center_trimmability", true);
  declareMeshProperty<bool>("peripheral_modifier_compatible", _pattern_boundary == "expanded");

  const unsigned int n_pattern_layers = _pattern.size();
  declareMeshProperty("pattern_size", n_pattern_layers);
  if (n_pattern_layers == 1)
    paramError("pattern", "The length (layer number) of this parameter must be larger than unity.");
  // Examine pattern for consistency
  std::vector<unsigned int> pattern_max_array;
  std::vector<unsigned int> pattern_1d;
  std::set<unsigned int> pattern_elem_size;
  for (const auto & pattern_elem : _pattern)
  {
    pattern_elem_size.emplace(pattern_elem.size());
    pattern_max_array.push_back(*std::max_element(pattern_elem.begin(), pattern_elem.end()));
    pattern_1d.insert(pattern_1d.end(), pattern_elem.begin(), pattern_elem.end());
  }
  if (pattern_elem_size.size() > 1 || *pattern_elem_size.begin() != _pattern.size())
    paramError("pattern",
               "The two-dimensional array parameter pattern must have a correct square shape.");

  if (*std::max_element(pattern_max_array.begin(), pattern_max_array.end()) >= _input_names.size())
    paramError(
        "pattern",
        "Elements of this parameter must be smaller than the length of inputs (0-indexing).");
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

  if (_pattern_boundary == "expanded")
  {
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

  if (_use_reporting_id)
  {
    if (_use_exclude_id && _assign_type != ReportingIDGeneratorUtils::AssignType::cell)
      paramError("exclude_id", "works only when \"assign_type\" is equal 'cell'");
    if (!isParamValid("id_pattern") &&
        _assign_type == ReportingIDGeneratorUtils::AssignType::manual)
      paramError("id_pattern", "required when \"assign_type\" is equal to 'manual'");

    if (_assign_type == ReportingIDGeneratorUtils::AssignType::manual)
      _id_pattern = getParam<std::vector<std::vector<dof_id_type>>>("id_pattern");
    _exclude_ids.resize(_input_names.size());
    // in case of using 'exclude_id', create a vector containg flag for each input tile to indicate
    // whether it is excluded from reporting id assignment
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
PatternedCartesianMeshGenerator::generate()
{
  std::vector<std::unique_ptr<ReplicatedMesh>> meshes(_input_names.size());
  for (const auto i : index_range(_input_names))
  {
    mooseAssert(_mesh_ptrs[i] && (*_mesh_ptrs[i]).get(), "nullptr mesh");
    meshes[i] = dynamic_pointer_cast<ReplicatedMesh>(std::move(*_mesh_ptrs[i]));
    if (!meshes[i])
      paramError("inputs", "Mesh '", _input_names[i], "' is not a replicated mesh but it must be");
    // throw an error message if the input mesh does not have a flat side up
    if (hasMeshProperty<bool>("flat_side_up", _input_names[i]))
      if (!getMeshProperty<bool>("flat_side_up", _input_names[i]))
        paramError("inputs",
                   "Mesh '",
                   _input_names[i],
                   "' does not have a flat side facing up, which is not supported.");
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
    // Extract & check pitch and drum metadata
    for (MooseIndex(_input_names) i = 0; i < _input_names.size(); ++i)
    {
      // throw an error message if the input mesh does not contain the required meta data
      if (!hasMeshProperty<Real>("pattern_pitch_meta", _input_names[i]))
        mooseError("In PatternedCartesianMeshGenerator ",
                   _name,
                   ": the unit square input mesh does not contain appropriate meta data "
                   "required for generating a core mesh. Involved input mesh: ",
                   _input_names[i],
                   "; metadata issue: 'pattern_pitch_meta' is missing.");
      pattern_pitch_array.push_back(getMeshProperty<Real>("pattern_pitch_meta", _input_names[i]));
      // throw an error message if the input mesh contains non-sense meta data
      if (pattern_pitch_array.back() == 0.0)
        mooseError("In PatternedCartesianMeshGenerator ",
                   _name,
                   ": the unit square input mesh does not contain appropriate meta data "
                   "required for generating a core mesh. Involved input mesh: ",
                   _input_names[i],
                   "; metadata issue: 'pattern_pitch_meta' is zero.");
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
      mooseError("In PatternedCartesianMeshGenerator ",
                 _name,
                 ": pattern_pitch metadata values of all input mesh generators must be identical "
                 "when pattern_boundary is 'none' and generate_core_metadata is true.");
    else
    {
      _pattern_pitch = pattern_pitch_array.front();
      setMeshProperty("input_pitch_meta", _pattern_pitch);
    }
  }
  else
  {
    if (_pattern_boundary == "expanded")
    {
      if (!isParamValid("square_size"))
        paramError("square_size",
                   "This parameter must be provided when pattern_boundary is expanded.");
      else
        _pattern_pitch = getParam<Real>("square_size");
    }
    else if (isParamValid("square_size"))
      _pattern_pitch = getParam<Real>("square_size");

    for (MooseIndex(_input_names) i = 0; i < _input_names.size(); ++i)
    {
      // throw an error message if the input mesh does not contain the required meta data
      if (!hasMeshProperty<Real>("pitch_meta", _input_names[i]))
        mooseError("In PatternedCartesianMeshGenerator ",
                   _name,
                   ": the unit square input mesh does not contain appropriate meta data "
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
        mooseError("In PatternedCartesianMeshGenerator ",
                   _name,
                   ": num_sectors_per_side metadata values of all four sides of each input mesh "
                   "generator must be identical.");
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
      mooseError("In PatternedCartesianMeshGenerator ",
                 _name,
                 ": pitch metadata values of all input mesh generators must be identical.");
    setMeshProperty("input_pitch_meta", pitch_array.front());
    if (*std::max_element(num_sectors_per_side_array.begin(), num_sectors_per_side_array.end()) !=
        *std::min_element(num_sectors_per_side_array.begin(), num_sectors_per_side_array.end()))
      mooseError(
          "In PatternedCartesianMeshGenerator ",
          _name,
          ": num_sectors_per_side metadata values of all input mesh generators must be identical.");
  }

  std::vector<Real> extra_dist;
  Real extra_dist_shift(0.0);
  Real y_min(0.0);
  Real y_max_0(0.0);
  Real y_max_n(0.0);
  const Real extra_dist_tol = _pattern_boundary == "expanded" ? pitch_array.front() / 10.0 : 0.0;
  const Real extra_dist_shift_0 = _pattern_boundary == "expanded" ? pitch_array.front() / 5.0 : 0.0;
  std::vector<unsigned int> peripheral_duct_intervals;
  if (_pattern_boundary == "expanded")
  {
    if (_has_assembly_duct)
      for (unsigned int i = 0; i < _duct_sizes.size(); i++)
      {
        if (_duct_sizes_style == PolygonSizeStyle::radius)
          _duct_sizes[i] *= std::cos(M_PI / 4.0);
        /// As square geometry is used here, size of patterned mesh can be straightforwardly calculated.
        extra_dist.push_back(0.5 * (_duct_sizes[i] * 2.0 - pitch_array.front() * _pattern.size()));
        peripheral_duct_intervals.push_back(_duct_intervals[i]);
      }
    // calculate the distance between the larger square boundary and the boundary of stitched unit
    // squares this is used to decide whether deformation is needed when cut-off happens or when
    // the distance is small.
    extra_dist.push_back(0.5 * (_pattern_pitch - pitch_array.front() * _pattern.size()));
    peripheral_duct_intervals.insert(peripheral_duct_intervals.begin(), _background_intervals);

    // In some cases, when the external square size is small enough, the external square boundary
    // may either be very close to the input square meshes that are near the boundary or even cut
    // off by these squares. As long as the ring regions are not cut off, the input squares can
    // be deformed to accomodate the external square shape. This block sets up the range of mesh
    // region that needs to be deformed.
    if (extra_dist.front() <= extra_dist_tol)
    {
      extra_dist_shift = extra_dist_shift_0 - extra_dist.front();
      for (Real & d : extra_dist)
        d += extra_dist_shift;
      y_min = _deform_non_circular_region
                  ? max_radius_global // Currently use this, ideally this should be the max of the
                                      // outer layer radii
                  : (pitch_array.front() / 2.0);
      y_max_0 = pitch_array.front() / 2.0 + extra_dist.front();
      y_max_n = y_max_0 - extra_dist_shift;
      if (y_max_n <= y_min)
        mooseError("In PatternedCartesianMeshGenerator ",
                   _name,
                   ": the assembly is cut off so much that the internal structure that should not "
                   "be altered is compromised.");
    }
  }

  setMeshProperty("pattern_pitch_meta", _pattern_pitch);

  const Real input_pitch((_pattern_boundary == "expanded" || !_generate_core_metadata)
                             ? pitch_array.front()
                             : _pattern_pitch);
  std::vector<Real> control_drum_positions_x;
  std::vector<Real> control_drum_positions_y;
  std::vector<std::vector<Real>> control_drum_azimuthals;

  std::unique_ptr<ReplicatedMesh> out_mesh;

  for (unsigned i = 0; i < _pattern.size(); i++)
  {
    const Real deltax = 0.0;
    Real deltay = -(Real)(i)*input_pitch;

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
          rotation_angle = 90.;
          mesh_type = CORNER_MESH;
          orientation = 0.;
        }
        else if (j == 0 && i == _pattern.size() - 1)
        {
          rotation_angle = 180.;
          mesh_type = CORNER_MESH;
          orientation = -90.;
        }
        else if (j == _pattern[i].size() - 1 && i == 0)
        {
          rotation_angle = 0.;
          mesh_type = CORNER_MESH;
          orientation = 90.;
        }
        else if (j == _pattern[i].size() - 1 && i == _pattern.size() - 1)
        {
          rotation_angle = -90.;
          mesh_type = CORNER_MESH;
          orientation = 180.;
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
        else if (j == 0)
        {
          rotation_angle = 90.;
          mesh_type = BOUNDARY_MESH;
          orientation = -90.;
        }
        else if (j == _pattern[i].size() - 1)
        {
          rotation_angle = -90.;
          mesh_type = BOUNDARY_MESH;
          orientation = 90;
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
            cutOffPolyDeform(
                *tmp_peripheral_mesh, orientation, y_max_0, y_max_n, y_min, mesh_type, 90.0);

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

      out_mesh->stitch_meshes(pattern_mesh,
                              OUTER_SIDESET_ID,
                              OUTER_SIDESET_ID,
                              TOLERANCE,
                              /*clear_stitched_boundary_ids=*/false,
                              /*verbose=*/false);

      // Translate back now that we've stitched so that anyone else that uses this mesh has it at
      // the origin
      MeshTools::Modification::translate(pattern_mesh, -(deltax + j * input_pitch), -deltay, 0);
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
    MeshTools::Modification::rotate(*out_mesh, -45.0, 0.0, 0.0);
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
    const unsigned int side_intervals = node_azi_list.size() / SQUARE_NUM_SIDES;
    for (unsigned int i = 0; i < SQUARE_NUM_SIDES; i++)
    {
      for (unsigned int j = 1; j <= side_intervals; j++)
      {
        Real azi_corr_tmp = atan2((Real)j * 2.0 / (Real)side_intervals - 1.0, 1.0);
        Real x_tmp = _pattern_pitch / 2.0;
        Real y_tmp = x_tmp * std::tan(azi_corr_tmp);
        nodeCoordRotate(x_tmp, y_tmp, (Real)i * 90.0 - 135.0);
        Point p_tmp = Point(x_tmp, y_tmp, 0.0);
        out_mesh->add_point(p_tmp, node_azi_list[i * side_intervals + j - 1].second);
      }
    }
    MeshTools::Modification::rotate(*out_mesh, 45.0, 0.0, 0.0);
  }

  MeshTools::Modification::rotate(*out_mesh, _rotate_angle, 0.0, 0.0);

  // This combination of input parameters is usually used for core mesh generation by stitching
  // assembly meshes together.
  if (_pattern_boundary == "none" && _generate_core_metadata)
  {
    const Real azi_tol = 1E-8;
    std::vector<std::tuple<Real, Point, std::vector<Real>, dof_id_type>> control_drum_tmp;
    std::vector<dof_id_type> control_drum_id_sorted;
    unsigned int drum_integer_index = out_mesh->get_elem_integer_index("control_drum_id");
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
            M_PI * 180.0; // quick way to move from -M_PI to M_PI
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
        dof_id_type unsorted_control_drum_id = elem->get_extra_integer(drum_integer_index);
        if (unsorted_control_drum_id != 0)
        {
          auto sorted_iter = std::find(control_drum_id_sorted.begin(),
                                       control_drum_id_sorted.end(),
                                       unsorted_control_drum_id);
          elem->set_extra_integer(drum_integer_index,
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

  auto mesh = dynamic_pointer_cast<MeshBase>(out_mesh);
  // before return, add reporting IDs if _use_reporting_id is set true
  if (_use_reporting_id)
    addReportingIDs(mesh, meshes);
  return mesh;
}

void
PatternedCartesianMeshGenerator::addPeripheralMesh(
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
    // corner mesh has two sides that need peripheral meshes.
    // each element has three sub-elements, representing beginning, middle, and ending azimuthal
    // points
    peripheral_point_index = {{0, 1, 2}, {2, 3, 4}};
  else
    // side mesh has one side that needs a peripheral mesh.
    peripheral_point_index = {{0, 1, 5}};

  // extra_dist includes background and ducts.
  // Loop to calculate the positions of the boundaries.
  for (unsigned int i = 0; i < extra_dist.size(); i++)
  {
    // Generate the node positions for the peripheral meshes
    // The node positions are generated for the two possible cases: the edge and the corner
    positionSetup(
        positions_inner, d_positions_outer, i == 0 ? 0.0 : extra_dist[i - 1], extra_dist[i], pitch);

    // Loop for all applicable sides that need peripheral mesh (2 for corner and 1 for edge)
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
PatternedCartesianMeshGenerator::positionSetup(
    std::vector<std::pair<Real, Real>> & positions_inner,
    std::vector<std::pair<Real, Real>> & d_positions_outer,
    const Real extra_dist_in,
    const Real extra_dist_out,
    const Real pitch) const
{
  positions_inner.resize(0);
  d_positions_outer.resize(0);
  // Nine sets of positions are generated here, as shown below.
  // CORNER MESH Peripheral {0 1 2} and {2 3 4}
  //
  //           0    1         2
  //           |    |        /
  //           |    |      /
  //           |____|____/
  //           |         |
  //           |         |____ 3
  //           |         |
  //           |_________|____ 4
  //
  // EDGE MESH Peripheral {0 1 5}
  //
  //           0    1    5
  //           |    |    |
  //           |    |    |
  //           |____|____|
  //           |         |
  //           |         |
  //           |         |
  //           |_________|

  // Inner positions defined from index 0 through 5 as shown in the above cartoon
  positions_inner.push_back(std::make_pair(-pitch / 2.0, pitch / 2.0 + extra_dist_in));
  positions_inner.push_back(std::make_pair(0.0, pitch / 2.0 + extra_dist_in));
  positions_inner.push_back(
      std::make_pair(pitch / 2.0 + extra_dist_in, pitch / 2.0 + extra_dist_in));
  positions_inner.push_back(std::make_pair(pitch / 2.0 + extra_dist_in, 0.0));
  positions_inner.push_back(std::make_pair(pitch / 2.0 + extra_dist_in, -pitch / 2.0));
  positions_inner.push_back(std::make_pair(pitch / 2.0, pitch / 2.0 + extra_dist_in));

  // Outer positions (relative displacement from inner ones) defined from index 0 through 5 as shown
  // in the above cartoon
  d_positions_outer.push_back(std::make_pair(0.0, extra_dist_out - extra_dist_in));
  d_positions_outer.push_back(std::make_pair(0.0, extra_dist_out - extra_dist_in));
  d_positions_outer.push_back(
      std::make_pair(extra_dist_out - extra_dist_in, extra_dist_out - extra_dist_in));
  d_positions_outer.push_back(std::make_pair(extra_dist_out - extra_dist_in, 0.0));
  d_positions_outer.push_back(std::make_pair(extra_dist_out - extra_dist_in, 0.0));
  d_positions_outer.push_back(std::make_pair(0.0, extra_dist_out - extra_dist_in));
}

void
PatternedCartesianMeshGenerator::addReportingIDs(
    std::unique_ptr<MeshBase> & mesh,
    const std::vector<std::unique_ptr<ReplicatedMesh>> & from_meshes) const
{
  unsigned int extra_id_index;
  const std::string element_id_name = getParam<std::string>("id_name");
  if (!mesh->has_elem_integer(element_id_name))
    extra_id_index = mesh->add_elem_integer(element_id_name);
  else
  {
    extra_id_index = mesh->get_elem_integer_index(element_id_name);
    paramWarning(
        "id_name", "An element integer with the name '", element_id_name, "' already exists");
  }

  // asssign reporting IDs to individual elements
  std::set<subdomain_id_type> background_block_ids;
  if (isParamValid("background_block_id"))
    background_block_ids.insert(getParam<subdomain_id_type>("background_block_id"));
  ReportingIDGeneratorUtils::assignReportingIDs(mesh,
                                                extra_id_index,
                                                _assign_type,
                                                _use_exclude_id,
                                                _exclude_ids,
                                                _pattern_boundary == "expanded",
                                                background_block_ids,
                                                from_meshes,
                                                _pattern,
                                                _id_pattern);
}
