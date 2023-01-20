//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedCurveGenerator.h"
#include "LinearInterpolation.h"
#include "MooseUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/fparser_ad.hh"
#include "libmesh/edge_edge2.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseApp", ParsedCurveGenerator);

InputParameters
ParsedCurveGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<std::string>("x_formula", "Function expression of x(t)");
  params.addRequiredParam<std::string>("y_formula", "Function expression of y(t)");
  params.addParam<std::string>("z_formula", "0", "Function expression of z(t)");
  params.addRequiredRangeCheckedParam<std::vector<unsigned int>>(
      "nums_segments",
      "nums_segments>=1",
      "Numbers of segments (EDGE2 elements) of each section of the curve to be generated. The "
      "number of entries in this parameter should be equal to one less than the number of entries "
      "in 'section_bounding_t_values'");
  params.addRequiredParam<std::vector<Real>>(
      "section_bounding_t_values",
      "The 't' values that bound the sections of the curve. Start and end points must be included. "
      "The number of entries in 'nums_segments' should be equal to one less than the number of "
      "entries in this parameter.");
  params.addParam<std::vector<std::string>>(
      "constant_names", "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addParam<bool>("is_closed_loop", false, "Whether the curve is closed or not.");
  params.addRangeCheckedParam<Real>("point_overlapping_tolerance",
                                    libMesh::TOLERANCE,
                                    "point_overlapping_tolerance>0.0",
                                    "The point-to-point distance tolerance that is used to "
                                    "determine whether the two points are overlapped.");
  params.addRangeCheckedParam<unsigned int>(
      "forced_closing_num_segments",
      "forced_closing_num_segments>1",
      "Number of segments (EDGE2 elements) of the curve section that is generated to forcefully "
      "close the loop.");
  params.addRangeCheckedParam<Real>("oversample_factor",
                                    10.0,
                                    "oversample_factor>2.0",
                                    "Oversample factor to help make node distance nearly uniform.");
  params.addRangeCheckedParam<unsigned int>(
      "max_oversample_number_factor",
      1000,
      "max_oversample_number_factor>100",
      "For each section of the curve, the maximum number of oversampling points is the product of "
      "this factor and the number of nodes on the curve.");

  params.addClassDescription("This ParsedCurveGenerator object is designed to generate a mesh of a "
                             "curve that consists of EDGE2 elements.");

  return params;
}

ParsedCurveGenerator::ParsedCurveGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    FunctionParserUtils<false>(parameters),
    _function_x(getParam<std::string>("x_formula")),
    _function_y(getParam<std::string>("y_formula")),
    _function_z(getParam<std::string>("z_formula")),
    _nums_segments(getParam<std::vector<unsigned int>>("nums_segments")),
    _section_bounding_t_values(getParam<std::vector<Real>>("section_bounding_t_values")),
    _is_closed_loop(getParam<bool>("is_closed_loop")),
    _point_overlapping_tolerance(getParam<Real>("point_overlapping_tolerance")),
    _forced_closing_num_segments(isParamValid("forced_closing_num_segments")
                                     ? getParam<unsigned int>("forced_closing_num_segments")
                                     : 0),
    _oversample_factor(getParam<Real>("oversample_factor")),
    _max_oversample_number_factor(getParam<unsigned int>("max_oversample_number_factor"))
{
  if (std::adjacent_find(_section_bounding_t_values.begin(), _section_bounding_t_values.end()) !=
      _section_bounding_t_values.end())
    paramError("section_bounding_t_values", "elements must be unique.");
  if (!std::is_sorted(_section_bounding_t_values.begin(), _section_bounding_t_values.end()) &&
      !std::is_sorted(_section_bounding_t_values.rbegin(), _section_bounding_t_values.rend()))
    paramError("section_bounding_t_values", "elements must change monotonically.");
  if (_nums_segments.size() != _section_bounding_t_values.size() - 1)
    paramError(
        "nums_segments",
        "The size of this parameter must be one less than size of section_bounding_t_values.");
  _func_Fx = std::make_shared<SymFunction>();
  _func_Fy = std::make_shared<SymFunction>();
  _func_Fz = std::make_shared<SymFunction>();

  // set FParser internal feature flags
  setParserFeatureFlags(_func_Fx);
  setParserFeatureFlags(_func_Fy);
  setParserFeatureFlags(_func_Fz);

  // add the constant expressions; note that the three functions share one set of constants
  addFParserConstants(_func_Fx,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"));
  addFParserConstants(_func_Fy,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"));
  addFParserConstants(_func_Fz,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"));

  // parse functions
  if (_func_Fx->Parse(_function_x, "t") >= 0)
    mooseError("Invalid function x(t)\n",
               _function_x,
               "\nin ParsedCurveGenerator ",
               name(),
               ".\n",
               _func_Fx->ErrorMsg());
  if (_func_Fy->Parse(_function_y, "t") >= 0)
    mooseError("Invalid function y(t)\n",
               _function_y,
               "\nin ParsedCurveGenerator ",
               name(),
               ".\n",
               _func_Fy->ErrorMsg());
  if (_func_Fz->Parse(_function_z, "t") >= 0)
    mooseError("Invalid function z(t)\n",
               _function_z,
               "\nin ParsedCurveGenerator ",
               name(),
               ".\n",
               _func_Fz->ErrorMsg());

  _func_params.resize(1);

  if (!_is_closed_loop && _forced_closing_num_segments > 0)
    paramError("forced_closing_num_segments",
               "this parameter is not needed if the curve to be generated is not a closed loop.");
}

std::unique_ptr<MeshBase>
ParsedCurveGenerator::generate()
{
  auto mesh = buildReplicatedMesh(2);

  // Do oversampling for each section of the curve as defined by "section_bounding_t_values"
  for (unsigned int i = 0; i < _nums_segments.size(); i++)
  {
    std::vector<Real> t_sect_space;
    std::vector<Real> dis_sect_space;
    tSectionSpaceDefiner(_section_bounding_t_values[i],
                         _section_bounding_t_values[i + 1],
                         t_sect_space,
                         dis_sect_space,
                         _nums_segments[i],
                         _is_closed_loop && _nums_segments.size() == 1,
                         _oversample_factor);
    if (i > 0)
    {
      // Remove the last t value and distance value of previous section to avert overlapping
      _t_space.pop_back();
      // Add the last distance value as this is a cumulative distance
      for (auto & dis_elem : dis_sect_space)
        dis_elem += _dis_space.back();
      _dis_space.pop_back();
    }
    _t_space.insert(_t_space.end(), t_sect_space.begin(), t_sect_space.end());
    _dis_space.insert(_dis_space.end(), dis_sect_space.begin(), dis_sect_space.end());
  }

  // Use linear interpolation to help achieve nearly uniform intervals for each section
  std::unique_ptr<LinearInterpolation> linear_t;
  linear_t = std::make_unique<LinearInterpolation>(_dis_space, _t_space);

  std::vector<Node *> nodes(std::accumulate(_nums_segments.begin(), _nums_segments.end(), 0) + 1);
  for (unsigned int i = 0; i < nodes.size(); i++)
  {
    _func_params[0] = linear_t->sample((Real)i);
    Point side_p = Point(evaluate(_func_Fx), evaluate(_func_Fy), evaluate(_func_Fz));
    nodes[i] = mesh->add_point(side_p, i);
  }

  // For a closed loop, need to check if the first and last Points are overlapped
  if (_is_closed_loop)
  {
    if (MooseUtils::absoluteFuzzyEqual(
            (*nodes.back() - *nodes.front()).norm(), 0.0, _point_overlapping_tolerance))
    {
      // Remove the overlapped nodes for a closed loop
      mesh->delete_node(nodes.back());
      nodes.resize(nodes.size() - 1);
      if (_forced_closing_num_segments > 0)
        paramError("forced_closing_num_segments",
                   "this parameter is not needed if the first and last points of the curve to be "
                   "generated are overlapped.");
    }
    else if (_forced_closing_num_segments > 1)
    {
      // Add extra nodes on the curve section used to forcefully close the loop
      const Point start_pt(*nodes.back());
      const Point end_pt(*nodes.front());
      const unsigned int num_nodes_0(nodes.size());
      for (unsigned int i = 1; i < _forced_closing_num_segments; i++)
      {
        Point side_p =
            start_pt + (end_pt - start_pt) * (Real)i / (Real)_forced_closing_num_segments;
        nodes.push_back(mesh->add_point(side_p, num_nodes_0 + i - 1));
      }
    }
  }

  for (unsigned int i = 0; i < nodes.size() - !_is_closed_loop; i++)
  {
    Elem * elem = mesh->add_elem(new Edge2);
    elem->set_node(0) = nodes[i];
    elem->set_node(1) = nodes[(i + 1) % nodes.size()];
    elem->subdomain_id() = 1;
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
ParsedCurveGenerator::tSectionSpaceDefiner(const Real t_start,
                                           const Real t_end,
                                           std::vector<Real> & t_sect_space,
                                           std::vector<Real> & dis_sect_space,
                                           unsigned int num_segments,
                                           const bool is_closed_loop,
                                           const Real oversample_factor)
{
  std::vector<Point> pt_sect_space;
  t_sect_space.push_back(t_start);
  t_sect_space.push_back(t_end);
  pt_sect_space.push_back(pointCalculator(t_start));
  pt_sect_space.push_back(pointCalculator(t_end));
  const Real total_distance =
      is_closed_loop
          ? euclideanDistance(pt_sect_space.front(), pointCalculator((t_start + t_end) / 2.0))
          : euclideanDistance(pt_sect_space.front(), pt_sect_space.back());
  if (MooseUtils::absoluteFuzzyEqual(total_distance, 0.0))
    paramError("section_bounding_t_values",
               "The curve has at least one cross, which is not supported.");
  const Real target_fine_interval = total_distance / (Real)num_segments / oversample_factor;
  unsigned int t_space_id = 0;
  dis_sect_space.push_back(0.0);
  // Use a maximum oversampling points number and a counter to avert infinite loop
  const unsigned int max_binary_search = _max_oversample_number_factor * (num_segments + 1);
  unsigned int binary_search_counter(0);
  // Use binary algorithm to make all the intervals smaller than the target value
  while (t_space_id + 1 < t_sect_space.size() && binary_search_counter <= max_binary_search)
  {
    binary_search_counter++;
    if (euclideanDistance(pt_sect_space[t_space_id], pt_sect_space[t_space_id + 1]) >
        target_fine_interval)
    {
      const Real new_t_value = (t_sect_space[t_space_id] + t_sect_space[t_space_id + 1]) / 2.0;
      t_sect_space.insert(t_sect_space.begin() + t_space_id + 1, new_t_value);
      pt_sect_space.insert(pt_sect_space.begin() + t_space_id + 1, pointCalculator(new_t_value));
    }
    else
    {
      dis_sect_space.push_back(
          dis_sect_space.back() +
          euclideanDistance(pt_sect_space[t_space_id], pt_sect_space[t_space_id + 1]));
      t_space_id++;
    }
  }
  if (binary_search_counter > max_binary_search)
    paramError(
        "max_oversample_number_factor",
        "Maximum oversampling points number has been exceeded. Please consider adding more t "
        "values into 'section_bounding_t_values' or increase 'max_oversample_number_factor'.");
  const Real total_dis_seg = dis_sect_space.back();
  // Normalization to make the normalized length of the curve equals to number of segments
  for (auto & dist : dis_sect_space)
    dist = dist / total_dis_seg * (Real)num_segments;
}

Point
ParsedCurveGenerator::pointCalculator(const Real t_param)
{
  _func_params[0] = t_param;
  return Point(evaluate(_func_Fx), evaluate(_func_Fy), evaluate(_func_Fz));
}

Real
ParsedCurveGenerator::euclideanDistance(const Point p1, const Point p2)
{
  return (p1 - p2).norm();
}
