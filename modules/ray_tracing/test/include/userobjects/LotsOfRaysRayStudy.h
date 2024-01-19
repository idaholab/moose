//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RepeatableRayStudyBase.h"

// Local includes
#include "BoundingBoxIntersectionHelper.h"

/**
 * A RayTracingStudy used for generating a lot of rays for testing purposes
 */
class LotsOfRaysRayStudy : public RepeatableRayStudyBase
{
public:
  LotsOfRaysRayStudy(const InputParameters & parameters);

  static InputParameters validParams();

  /**
   * Whether or not the expected distance is being computed
   */
  bool hasExpectedDistance() const { return _compute_expected_distance; }
  /**
   * Get the expected total distance Rays should travel
   */
  Real expectedDistance() const { return _expected_distance; }

protected:
  virtual void defineRays() override;

  void defineRay(const Elem * starting_elem,
                 const unsigned short incoming_side,
                 const Point & p1,
                 const Point & p2,
                 const bool ends_within_mesh);

  /**
   * Insertion point for after _rays is defined for other derived test studies
   * to modify the Rays
   */
  virtual void modifyRays();

  const bool _vertex_to_vertex;
  const bool _centroid_to_vertex;
  const bool _centroid_to_centroid;
  const bool _edge_to_edge;
  const bool _side_aq;
  const bool _centroid_aq;

  /// Whether or not to compute the expected distance for generated rays
  const bool _compute_expected_distance;

  /// Polar angular quadrature order for aq tests
  const unsigned int _polar_quad_order;
  /// Azimuthal angular quadrature order for aq tests
  const unsigned int _azimuthal_quad_order;

  const bool _use_unsized_rays;
  const bool _set_incoming_side;

private:
  /// The expected total distance Rays should travel
  Real & _expected_distance;

  /// Helper for computing the end point for Rays that don't end within mesh
  std::unique_ptr<BoundingBoxIntersectionHelper> _bbox_intersection_helper;
};
