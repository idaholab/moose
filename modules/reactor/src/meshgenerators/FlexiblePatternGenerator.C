//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlexiblePatternGenerator.h"

// C++ includes
#include <cmath>

registerMooseObject("ReactorApp", FlexiblePatternGenerator);

InputParameters
FlexiblePatternGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();

  params.addRequiredParam<std::vector<MeshGeneratorName>>("inputs", "The input MeshGenerators.");
  MooseEnum boundary_type("HEXAGON CARTESIAN CIRCLE CUSTOM", "CUSTOM");
  params.addParam<MooseEnum>("boundary_type",
                             boundary_type,
                             "what type of boundary is used as background for patterning.");
  params.addParam<MeshGeneratorName>(
      "boundary_mesh",
      "The boundary mesh consisting of EDGE2 elements to be used as the 'CUSTOM' boundary.");
  params.addRangeCheckedParam<unsigned int>(
      "boundary_sectors",
      "boundary_sectors>0",
      "The number of sectors on each side of the HEXAGON or CARTESIAN boundary mesh or on the "
      "circular boundary of the CIRCLE boundary mesh.");
  params.addRangeCheckedParam<Real>("boundary_size",
                                    "boundary_size>0",
                                    "The pitch size of the HEXAGON or CARTESIAN boundary mesh; or "
                                    "the diameter of the CIRCLE boundary mesh.");

  params.addParam<std::vector<Point>>(
      "extra_positions", "The extra non-patterned positions to set the input MeshGenerators.");
  params.addParam<std::vector<unsigned int>>(
      "extra_positions_mg_indices",
      "the indices of the input mesh generators for the extra position.");

  params.addParam<std::vector<std::vector<std::vector<unsigned int>>>>("hex_patterns",
                                                                       "Hexagonal patterns set.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "hex_pitches", "hex_pitches>0", "pitch sizes used to generate the hexagonal patterns.");
  params.addParam<std::vector<Point>>("hex_origins",
                                      "the origin positions of the hexagonal patterns,");
  params.addParam<std::vector<Real>>("hex_rotations",
                                     "the rotation angles of the hexagonal patterns,");

  params.addParam<std::vector<std::vector<std::vector<unsigned int>>>>("rect_patterns",
                                                                       "Rectangular patterns set.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "rect_pitches_x",
      "rect_pitches_x>0",
      "pitch sizes in x direction used to generate the rectangular patterns.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "rect_pitches_y",
      "rect_pitches_y>0",
      "pitch sizes in y direction used to generate the rectangular patterns.");
  params.addParam<std::vector<Point>>("rect_origins",
                                      "the origin positions of the rectangular patterns,");
  params.addParam<std::vector<Real>>("rect_rotations",
                                     "the rotation angles of the rectangular patterns.");

  params.addParam<std::vector<std::vector<unsigned int>>>("circular_patterns",
                                                          "Circular patterns set.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "circular_radii", "circular_radii>0", "the radii of the circular patterns.");
  params.addParam<std::vector<Point>>("circular_origins",
                                      "the origin positions of the circular patterns,");
  params.addParam<std::vector<Real>>(
      "circular_rotations",
      "the rotation angles of the circular patterns (the azimuthal angle of the first unit mesh).");

  // Parameters directly passed to XYDelaunayMeshGenerator
  params.addParam<Real>("desired_area", 0.0, "Desired are for the background area meshing.");
  params.addParam<std::string>(
      "desired_area_func",
      std::string(),
      "Desired area as a function of x,y; omit to skip non-uniform refinement");
  params.addParam<bool>("verify_holes", true, "Whether the holes are verified.");

  params.addParam<boundary_id_type>(
      "external_boundary_id",
      "The boundary id of the external boundary in addition to the default 10000.");

  params.addClassDescription("This FlexiblePatternGenerator object is designed to generate a "
                             "mesh with a background region with dispersed unit meshes in "
                             "it and distributed based on a series of flexible patterns.");

  params.addParamNamesToGroup("hex_patterns hex_pitches hex_origins hex_rotations",
                              "Hexagonal Pattern");
  params.addParamNamesToGroup(
      "rect_patterns rect_pitches_x rect_pitches_y rect_origins rect_rotations",
      "Rectangular Pattern");
  params.addParamNamesToGroup(
      "circular_patterns circular_radii circular_origins circular_rotations", "Circular Pattern");
  params.addParamNamesToGroup("extra_positions extra_positions_mg_indices",
                              "Extra Positions (Free-Style Patterns)");
  params.addParamNamesToGroup("desired_area desired_area_func verify_holes",
                              "Background Area Delaunay");

  return params;
}

FlexiblePatternGenerator::FlexiblePatternGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _boundary_type(getParam<MooseEnum>("boundary_type").template getEnum<BdryType>()),
    _boundary_mesh_name(isParamValid("boundary_mesh") ? getParam<MeshGeneratorName>("boundary_mesh")
                                                      : MeshGeneratorName()),
    _boundary_sectors(isParamValid("boundary_sectors") ? getParam<unsigned int>("boundary_sectors")
                                                       : 0),
    _boundary_size(isParamValid("boundary_size") ? getParam<Real>("boundary_size") : 0.0),
    _hex_patterns(
        isParamValid("hex_patterns")
            ? getParam<std::vector<std::vector<std::vector<unsigned int>>>>("hex_patterns")
            : std::vector<std::vector<std::vector<unsigned int>>>()),
    _hex_pitches(isParamValid("hex_pitches") ? getParam<std::vector<Real>>("hex_pitches")
                                             : std::vector<Real>()),
    _hex_origins(isParamValid("hex_origins")
                     ? getParam<std::vector<Point>>("hex_origins")
                     : std::vector<Point>(_hex_patterns.size(), Point(0.0, 0.0, 0.0))),
    _hex_rotations(isParamValid("hex_rotations") ? getParam<std::vector<Real>>("hex_rotations")
                                                 : std::vector<Real>(_hex_patterns.size(), 0.0)),
    _rect_patterns(
        isParamValid("rect_patterns")
            ? getParam<std::vector<std::vector<std::vector<unsigned int>>>>("rect_patterns")
            : std::vector<std::vector<std::vector<unsigned int>>>()),
    _rect_pitches_x(isParamValid("rect_pitches_x") ? getParam<std::vector<Real>>("rect_pitches_x")
                                                   : std::vector<Real>()),
    _rect_pitches_y(isParamValid("rect_pitches_y") ? getParam<std::vector<Real>>("rect_pitches_y")
                                                   : std::vector<Real>()),
    _rect_origins(isParamValid("rect_origins")
                      ? getParam<std::vector<Point>>("rect_origins")
                      : std::vector<Point>(_rect_patterns.size(), Point(0.0, 0.0, 0.0))),
    _rect_rotations(isParamValid("rect_rotations") ? getParam<std::vector<Real>>("rect_rotations")
                                                   : std::vector<Real>(_rect_patterns.size(), 0.0)),
    _circ_patterns(isParamValid("circular_patterns")
                       ? getParam<std::vector<std::vector<unsigned int>>>("circular_patterns")
                       : std::vector<std::vector<unsigned int>>()),
    _circ_radii(isParamValid("circular_radii") ? getParam<std::vector<Real>>("circular_radii")
                                               : std::vector<Real>()),
    _circ_origins(isParamValid("circular_origins")
                      ? getParam<std::vector<Point>>("circular_origins")
                      : std::vector<Point>(_circ_patterns.size(), Point(0.0, 0.0, 0.0))),
    _circ_rotations(isParamValid("circular_rotations")
                        ? getParam<std::vector<Real>>("circular_rotations")
                        : std::vector<Real>(_circ_patterns.size(), 0.0))
{
  declareMeshesForSub("inputs");

  const std::vector<Point> extra_positions(getParam<std::vector<Point>>("extra_positions"));
  const std::vector<unsigned int> extra_positions_mg_indices(
      getParam<std::vector<unsigned int>>("extra_positions_mg_indices"));
  if (extra_positions.size() != extra_positions_mg_indices.size())
    paramError("extra_positions_mg_indices",
               "This parameter must have the same size as extra_positions.");
  std::vector<unsigned int> input_usage_count(_input_names.size(), 0);
  for (unsigned int i = 0; i < extra_positions.size(); i++)
  {
    if (extra_positions_mg_indices[i] >= _input_names.size())
      paramError("extra_positions_mg_indices",
                 "the index used for extra positions must be available in 'inputs'.");
    input_usage_count[extra_positions_mg_indices[i]]++;
    _positions.push_back(std::make_pair(extra_positions[i], extra_positions_mg_indices[i]));
  }

  if (_boundary_type == BdryType::CUSTOM)
  {
    if (_boundary_mesh_name.empty())
      paramError("boundary_mesh", "boundary_mesh must be specified for CUSTOM boundary_type.");
    declareMeshForSub("boundary_mesh");
    if (_boundary_sectors > 0)
      paramError("boundary_sectors",
                 "this parameter should not be provided for CUSTOM boundary_type.");
    if (_boundary_size > 0.0)
      paramError("boundary_size",
                 "this parameter should not be provided for CUSTOM boundary_type.");
  }
  else
  {
    if (!_boundary_mesh_name.empty())
      paramError("boundary_mesh",
                 "this parameter should not be provided for non-CUSTOM "
                 "boundary_type.");
    if (_boundary_sectors == 0)
      paramError("boundary_sectors",
                 "this parameter must be provided for non-CUSTOM "
                 "boundary_type.");
    if (_boundary_size == 0.0)
      paramError("boundary_size", "this parameter must be provided for non-CUSTOM boundary_type.");

    if (_boundary_type == BdryType::HEXAGON)
    {
      _boundary_mesh_name = name() + "_hexagon_boundary";
      // create a submeshgenerator for the hexagon boundary
      auto params = _app.getFactory().getValidParams("PolyLineMeshGenerator");
      params.set<bool>("loop") = true;
      params.set<unsigned int>("num_edges_between_points") = _boundary_sectors;
      params.set<std::vector<Point>>("points") = {
          Point(0.0, _boundary_size / std::sqrt(3.0), 0.0),
          Point(_boundary_size / 2.0, _boundary_size / std::sqrt(3.0) / 2.0, 0.0),
          Point(_boundary_size / 2.0, -_boundary_size / std::sqrt(3.0) / 2.0, 0.0),
          Point(0.0, -_boundary_size / std::sqrt(3.0), 0.0),
          Point(-_boundary_size / 2.0, -_boundary_size / std::sqrt(3.0) / 2.0, 0.0),
          Point(-_boundary_size / 2.0, _boundary_size / std::sqrt(3.0) / 2.0, 0.0)};

      addMeshSubgenerator("PolyLineMeshGenerator", _boundary_mesh_name, params);
    }
    else if (_boundary_type == BdryType::CARTESIAN)
    {
      _boundary_mesh_name = name() + "_cartesian_boundary";
      // create a submeshgenerator for the cartesian boundary
      auto params = _app.getFactory().getValidParams("PolyLineMeshGenerator");
      params.set<bool>("loop") = true;
      params.set<unsigned int>("num_edges_between_points") = _boundary_sectors;
      params.set<std::vector<Point>>("points") = {
          Point(_boundary_size / 2.0, _boundary_size / 2.0, 0.0),
          Point(_boundary_size / 2.0, -_boundary_size / 2.0, 0.0),
          Point(-_boundary_size / 2.0, -_boundary_size / 2.0, 0.0),
          Point(-_boundary_size / 2.0, _boundary_size / 2.0, 0.0)};

      addMeshSubgenerator("PolyLineMeshGenerator", _boundary_mesh_name, params);
    }
    else
    {
      _boundary_mesh_name = name() + "_circle_boundary";
      // create a submeshgenerator for the circle boundary
      // As we are inducing polygonization anyway, PolyLineMeshGenerator is used
      auto params = _app.getFactory().getValidParams("PolyLineMeshGenerator");
      params.set<bool>("loop") = true;
      params.set<unsigned int>("num_edges_between_points") = 1;
      // We enforce radius correction here for area preservation
      const Real corr_factor =
          2 * M_PI / (Real)_boundary_sectors / std::sin(2 * M_PI / (Real)_boundary_sectors);
      std::vector<Point> circular_points;
      for (unsigned int i = 0; i < _boundary_sectors; i++)
      {
        const Real angle = 2.0 * M_PI * (Real)i / (Real)_boundary_sectors;
        circular_points.push_back(Point(_boundary_size * corr_factor * std::cos(angle) / 2.0,
                                        _boundary_size * corr_factor * std::sin(angle) / 2.0,
                                        0.0));
      }
      params.set<std::vector<Point>>("points") = circular_points;

      addMeshSubgenerator("PolyLineMeshGenerator", _boundary_mesh_name, params);
    }
    // Set metadata of an assembly mesh
    declareMeshProperty("pattern_pitch_meta", _boundary_size);
    declareMeshProperty<bool>("is_control_drum_meta", false);
  }

  // Hexagonal Pattern
  if (_hex_pitches.size() != _hex_patterns.size())
    paramError("hex_pitches",
               "The length of this parameter must be the same as that of hex_patterns.");
  if (_hex_origins.size() != _hex_patterns.size())
    paramError(
        "hex_origins",
        "if provided, the length of this parameter must be the same as that of hex_patterns.");
  if (_hex_rotations.size() != _hex_patterns.size())
    paramError(
        "hex_rotations",
        "if provided, the length of this parameter must be the same as that of hex_patterns.");
  std::vector<Point> hex_positions;
  if (!_hex_patterns.empty())
  {
    unsigned int hex_index = 0;
    for (const auto & hex_pattern : _hex_patterns)
    {
      const unsigned int n_hex_pattern_layers = hex_pattern.size();
      if (n_hex_pattern_layers % 2 == 0)
        paramError("hex_patterns",
                   "The length (layer number) of each element of this parameter must be odd to "
                   "ensure hexagonal shapes.");
      if (n_hex_pattern_layers == 1)
        paramError("hex_patterns",
                   "The length (layer number) of each element of this parameter must be larger "
                   "than unity.");
      for (unsigned int i = 0; i <= n_hex_pattern_layers / 2; i++)
      {
        if (hex_pattern[i].size() != n_hex_pattern_layers / 2 + i + 1 ||
            hex_pattern[n_hex_pattern_layers - 1 - i].size() != n_hex_pattern_layers / 2 + i + 1)
          paramError("hex_patterns",
                     "The two-dimentional array element of this parameter must have a correct "
                     "hexagonal shape.");
      }

      const Point unit_shift_1 = Point(_hex_pitches[hex_index], 0.0, 0.0);
      const Point unit_shift_2 =
          Point(_hex_pitches[hex_index] / 2.0, _hex_pitches[hex_index] / 2.0 * std::sqrt(3.0), 0.0);

      for (unsigned int i = 0; i < hex_pattern.size(); i++)
      {
        const Real param_2 = ((Real)hex_pattern.size() - 1.0) / 2.0 - (Real)i;
        const Real param_1_init = -((Real)hex_pattern.size() - 1.0) / 2.0 -
                                  ((i <= (hex_pattern.size() - 1) / 2)
                                       ? 0.0
                                       : (((Real)(hex_pattern.size() - 1) / 2) - (Real)i));
        for (unsigned int j = 0; j < hex_pattern[i].size(); j++)
        {
          // Any numbers exceeding the size of inputs are used as dummy units
          if (hex_pattern[i][j] < _input_names.size())
          {
            input_usage_count[hex_pattern[i][j]]++;
            Point pt_buffer = unit_shift_1 * (param_1_init + (Real)j) + unit_shift_2 * param_2;
            nodeCoordRotate(pt_buffer(0), pt_buffer(1), _hex_rotations[hex_index]);
            _positions.push_back(
                std::make_pair(pt_buffer + _hex_origins[hex_index], hex_pattern[i][j]));
          }
        }
      }
      hex_index++;
    }
  }

  // Rectangular Pattern
  if (_rect_pitches_x.size() != _rect_patterns.size())
    paramError("rect_pitches_x",
               "The length of this parameter must be the same as that of rect_patterns.");
  if (_rect_pitches_y.size() != _rect_patterns.size())
    paramError("rect_pitches_y",
               "The length of this parameter must be the same as that of rect_patterns.");
  if (_rect_origins.size() != _rect_patterns.size())
    paramError(
        "rect_origins",
        "if provided, the length of this parameter must be the same as that of rect_patterns.");
  if (_rect_rotations.size() != _rect_patterns.size())
    paramError(
        "rect_rotations",
        "if provided, the length of this parameter must be the same as that of rect_patterns.");
  if (!_rect_patterns.empty())
  {
    unsigned int rect_index = 0;
    for (const auto & rect_pattern : _rect_patterns)
    {
      std::set<unsigned int> rect_pattern_elem_size;
      for (const auto & rect_pattern_elem : rect_pattern)
        rect_pattern_elem_size.emplace(rect_pattern_elem.size());
      if (rect_pattern_elem_size.size() > 1)
        paramError("rect_patterns",
                   "The two-dimensional array element of this parameter must have a correct "
                   "rectangular shape.");

      const Point unit_shift_1 = Point(_rect_pitches_x[rect_index], 0.0, 0.0);
      const Point unit_shift_2 = Point(0.0, _rect_pitches_y[rect_index], 0.0);

      for (unsigned int i = 0; i < rect_pattern.size(); i++)
      {
        const Real param_2 = ((Real)rect_pattern.size() - 1.0) / 2.0 - (Real)i;
        const Real param_1_init = -((Real)rect_pattern[i].size() - 1.0) / 2.0;
        for (unsigned int j = 0; j < rect_pattern[i].size(); j++)
        {
          if (rect_pattern[i][j] < _input_names.size())
          {
            input_usage_count[rect_pattern[i][j]]++;
            Point pt_buffer = unit_shift_1 * (param_1_init + (Real)j) + unit_shift_2 * param_2;
            nodeCoordRotate(pt_buffer(0), pt_buffer(1), _rect_rotations[rect_index]);
            _positions.push_back(
                std::make_pair(pt_buffer + _rect_origins[rect_index], rect_pattern[i][j]));
          }
        }
      }
      rect_index++;
    }
  }

  // Circular Pattern
  if (_circ_radii.size() != _circ_patterns.size())
    paramError("circular_radii",
               "The length of this parameter must be the same as that of circular_patterns.");
  if (_circ_origins.size() != _circ_patterns.size())
    paramError(
        "circular_origins",
        "if provided, the length of this parameter must be the same as that of circular_patterns.");
  if (_circ_rotations.size() != _circ_patterns.size())
    paramError(
        "circular_rotations",
        "if provided, the length of this parameter must be the same as that of circular_patterns.");
  if (!_circ_patterns.empty())
  {
    unsigned int circ_index = 0;
    for (const auto & circ_pattern : _circ_patterns)
    {
      const Real angle_step = 2.0 * M_PI / (Real)circ_pattern.size();

      for (unsigned int i = 0; i < circ_pattern.size(); i++)
      {
        if (circ_pattern[i] < _input_names.size())
        {
          input_usage_count[circ_pattern[i]]++;
          Point pt_buffer = Point(_circ_radii[circ_index] * std::cos((Real)i * angle_step),
                                  _circ_radii[circ_index] * std::sin((Real)i * angle_step),
                                  0.0);
          nodeCoordRotate(pt_buffer(0), pt_buffer(1), _circ_rotations[circ_index]);
          _positions.push_back(
              std::make_pair(pt_buffer + _circ_origins[circ_index], circ_pattern[i]));
        }
      }
      circ_index++;
    }
  }

  if (std::count(input_usage_count.begin(), input_usage_count.end(), 0))
    paramError("inputs", "All the input mesh generator names are not used.");

  std::vector<MeshGeneratorName> patterned_pin_mg_series;
  for (unsigned int i = 0; i < _positions.size(); i++)
  {
    auto params = _app.getFactory().getValidParams("TransformGenerator");
    params.set<MeshGeneratorName>("input") = _input_names[_positions[i].second];
    params.set<MooseEnum>("transform") = 1;
    params.set<RealVectorValue>("vector_value") = _positions[i].first;

    patterned_pin_mg_series.push_back(name() + "_pos_" + std::to_string(i));

    addMeshSubgenerator("TransformGenerator", patterned_pin_mg_series.back(), params);
  }

  auto params = _app.getFactory().getValidParams("XYDelaunayGenerator");
  params.set<MeshGeneratorName>("boundary") = _boundary_mesh_name;
  params.set<std::vector<MeshGeneratorName>>("holes") = patterned_pin_mg_series;
  params.set<bool>("refine_boundary") = false;
  // XYDelaunay's intrinsic checks
  params.set<bool>("verify_holes") = getParam<bool>("verify_holes");
  params.set<std::vector<bool>>("stitch_holes") =
      std::vector<bool>(patterned_pin_mg_series.size(), true);
  params.set<std::vector<bool>>("refine_holes") =
      std::vector<bool>(patterned_pin_mg_series.size(), false);
  params.set<Real>("desired_area") = getParam<Real>("desired_area");
  params.set<std::string>("desired_area_func") = getParam<std::string>("desired_area_func");
  params.set<BoundaryName>("output_boundary") = std::to_string(OUTER_SIDESET_ID);
  addMeshSubgenerator("XYDelaunayGenerator", name() + "_pattern", params);

  _build_mesh = &getMeshByName(name() + "_pattern");
}

std::unique_ptr<MeshBase>
FlexiblePatternGenerator::generate()
{
  if (isParamValid("external_boundary_id"))
    MooseMesh::changeBoundaryId(
        **_build_mesh, OUTER_SIDESET_ID, getParam<boundary_id_type>("external_boundary_id"), false);
  (*_build_mesh)->find_neighbors();
  (*_build_mesh)->set_isnt_prepared();
  return std::move(*_build_mesh);
}
