//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

class ParsedCurveGenerator;

/**
 * Snaps the nodes of a boundary onto the parametric curve of a ParsedCurveGenerator.
 *
 * A mesh generated from a curve is bounded by the chords between the points of that curve, so its
 * boundary nodes only approximate the curve. Moving every node of the boundary to the closest
 * point of the curve recovers the geometry that those chords approximate. The curve is used in
 * the XY plane, so only the x and y coordinates of the nodes are snapped.
 */
class ParsedCurveNodeSnapGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  ParsedCurveNodeSnapGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Reference to the input mesh pointer, whose boundary nodes are snapped onto the curve
  std::unique_ptr<MeshBase> & _input;
  /// Reference to the mesh pointer of the curve generator, which is requested for its dependency
  std::unique_ptr<MeshBase> & _curve_mesh;
  /// The ParsedCurveGenerator that defines the curve to snap the nodes onto, and evaluates it
  ParsedCurveGenerator & _curve_generator;
  /// Parameters of the ParsedCurveGenerator that define the curve to snap the nodes onto
  const InputParameters & _curve_params;
  /// Name of the boundary whose nodes are snapped onto the curve
  const BoundaryName _boundary_name;
  /// Number of uniform samples of each curve section used to bracket the closest curve point
  const unsigned int _samples_per_section;
  /// t values that bound the sections of the curve
  const std::vector<Real> _section_bounding_t_values;
  /// Whether the curve is a closed loop, in which case the parameter wraps at the bounding values
  const bool _is_closed_loop;
  /// Sampled t values used to bracket the closest curve point
  std::vector<Real> _t_samples;

  /**
   * Gets the generator that defines the curve, after checking that it is a ParsedCurveGenerator
   * @return The referenced mesh generator
   */
  ParsedCurveGenerator & curveGenerator() const;

  /**
   * Evaluates the curve through the generator that defines it. The nodes are only snapped in the
   * XY plane, so a z coordinate of the curve is left out of everything done with the point here.
   * @param t_param Parameter t of the curve, which is wrapped into the bounding t values if the
   * curve is a closed loop
   * @return The point coordinates on the curve
   */
  Point curvePoint(const Real t_param);

  /**
   * Calculates the squared in-plane distance between a given point and a point of the curve
   * @param t_param Parameter t that defines the point of the curve
   * @param point The point to measure the distance from
   * @return The squared distance in the XY plane
   */
  Real squaredDistance(const Real t_param, const Point & point);

  /**
   * Finds the parameter t of the curve point that is the closest to a given point. The sampled t
   * values bracket the closest point and the bracket is then refined with a golden-section search.
   * @param point The point to project onto the curve
   * @return The parameter t of the closest point of the curve
   */
  Real closestParameter(const Point & point);

  /**
   * Refines a bracket of the closest curve point with a golden-section search
   * @param t_lower Lower bound of the bracketing t values
   * @param t_upper Upper bound of the bracketing t values
   * @param point The point to project onto the curve
   * @return The parameter t of the closest point of the curve within the bracket
   */
  Real goldenSectionSearch(const Real t_lower, const Real t_upper, const Point & point);

  /**
   * Wraps a parameter t into the bounding t values of a closed loop, which is where the formulas
   * of the curve define it
   * @param t_param Parameter t of the curve
   * @return The equivalent parameter t within the bounding t values
   */
  Real wrappedParameter(const Real t_param) const;
};
