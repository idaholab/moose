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
#include "MooseEnum.h"
#include "MooseMeshUtils.h"

/**
 * This CircularBoundaryCorrectionGenerator object is designed to correct full or partial circular
 * boundaries in a 2D mesh to preserve areas.
 */
class CircularBoundaryCorrectionGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  CircularBoundaryCorrectionGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Name of the mesh generator to get the input mesh
  const MeshGeneratorName _input_name;
  /// Names of the circular boundaries of the 2D input mesh to correct
  const std::vector<BoundaryName> _input_mesh_circular_boundaries;
  /// Ratio parameters used to customize the transition area sizes
  const std::vector<Real> _transition_layer_ratios;
  /// Customized tolerance used to verify that boundaries are circular
  const std::vector<Real> _custom_circular_tolerance;
  /// IDs of the circular boundary of the input mesh
  std::vector<boundary_id_type> _input_mesh_circular_bids;
  /// Whether to move the end nodes of the partial circular boundary in the span direction
  const bool _move_end_nodes_in_span_direction;
  /// Reference to input mesh pointer
  std::unique_ptr<MeshBase> & _input;

  /**
   * Calculates the center of the circle/partial circle formed by a vector of points
   * @param pts_list list of points on the circular boundary
   * @param radius a reference to a variable to contain the radius of the circular boundary to be
   * calculated
   * @param tol tolerance used to verify whether the boundary is circular or not
   * @return center of the circular boundary
   */
  Point circularCenterCalculator(const std::vector<Point> & pts_list,
                                 Real & radius,
                                 const Real tol = 1e-12) const;

  /**
   * Calculates the radius correction factor based on a list of sides on a circular boundary
   * @param bd_side_list list of sides on the circular boundary
   * @param circle_center center of the circular boundary
   * @param is_closed_loop whether the boundary is a closed loop or not
   * @param move_end_nodes_in_span_direction whether to move the end nodes of the partial circular
   * boundary in the span direction
   * @param c_coeff a reference to a variable to contain the coefficient that multiplies the
   * azimuthal angles
   * @param end_node_disp a reference to a variable to contain the displacement of the end node of
   * the partial circular boundary
   * @return radius correction factor to preserve circular area
   */
  Real generateRadialCorrectionFactor(const std::vector<std::pair<Point, Point>> & bd_side_list,
                                      const Point & circle_center,
                                      const bool is_closed_loop,
                                      const bool move_end_nodes_in_span_direction,
                                      Real & c_coeff,
                                      Real & end_node_disp) const;

  /**
   * Calculates the summation of the sine values of a list of angles
   * @param th_list list of angles
   * @param coeff coefficient before the angles inside the sine function
   * @return summation of the sine values of the angles
   */
  Real sineSummation(const std::vector<Real> & th_list, const Real coeff) const;

  /**
   * Calculates the first derivative with regards to coeff of the summation of the sine values of a
   * list of angles
   * @param th_list list of angles
   * @param coeff coefficient before the angles inside the sine function
   * @return first derivative of the summation of the sine values of the angles
   */
  Real sinePrimeSummation(const std::vector<Real> & th_list, const Real coeff) const;
};
