//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "MeshGenerator.h"

#include "FunctionParserUtils.h"

/**
 * his ParsedCurveGenerator object is designed to generate a mesh of a curve that consists of EDGE2
 * elements.
 */
class ParsedCurveGenerator : public MeshGenerator, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedCurveGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  /**
   * Calculates the oversampled parameter t series and corresponding cumulative distances from the
   * starting point of a section of a curve
   * @param t_start starting value of parameter t
   * @param t_end ending value of parameter t
   * @param t_sect_space a vector to store oversampled t value series
   * @param dis_sect_space a vector to store oversampled cumulative distance series
   * @param num_segments number of EDGE2 elements defined on the section of the curve
   * @param is_closed_loop whether the section is a closed loop
   * @param oversample_factor oversampling factor used to help make each EDGE2 element has similar
   * length
   */
  void tSectionSpaceDefiner(const Real t_start,
                            const Real t_end,
                            std::vector<Real> & t_sect_space,
                            std::vector<Real> & dis_sect_space,
                            unsigned int num_segments,
                            const bool is_closed_loop,
                            const Real oversample_factor);

protected:
  /// function expression for x(t)
  const std::string _function_x;
  /// function expression for y(t)
  const std::string _function_y;
  /// function expression for z(t)
  const std::string _function_z;
  /// numbers of side segments of each section defined by section_bounding_t_values
  const std::vector<unsigned int> _nums_segments;
  /// critical t values that define the sections of the curve
  const std::vector<Real> _section_bounding_t_values;
  /// whether the curve is a closed loop or not
  const bool _is_closed_loop;
  /// the point-to-point distance tolerance that is used to determine whether the two points are overlapped.
  const Real _point_overlapping_tolerance;
  /// Number of segments of the curve section that is generated to forcefully close the loop.
  const unsigned int _forced_closing_num_segments;
  /// Oversampling factor to help make node distance nearly uniform
  const Real _oversample_factor;
  /// A factor used to calculate the maximum oversampling points number in each section
  const unsigned int _max_oversample_number_factor;
  /// t values that are sampled for curve points
  std::vector<Real> _t_space;
  /// cumulative distances of the curve points from the starting ppint
  std::vector<Real> _dis_space;
  /// function parser object describing the x(t)
  SymFunctionPtr _func_Fx;
  /// function parser object describing the y(t)
  SymFunctionPtr _func_Fy;
  /// function parser object describing the z(t)
  SymFunctionPtr _func_Fz;

  /**
   * Calculates the point coordinates {x(t), y(t), 0.0} based on parameter t
   * @param t_param parameter t that is used to determine the coordinates of the point
   * @return the point coordinates
   */
  Point pointCalculator(const Real t_param);

  /**
   * Calculates the Euclidean distance between two given points
   * @param p1 the first point used to calculate the distance
   * @param p2 the second point used to calculate the distance
   * @return the Euclidean distance between the two given points
   */
  Real euclideanDistance(const Point p1, const Point p2);

  usingFunctionParserUtilsMembers(false);
};
