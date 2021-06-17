//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Marker.h"

/**
 * Marks all elements near a given boundary for refinement/coarsening
 */
class BoundaryMarker : public Marker
{
public:
  static InputParameters validParams();

  BoundaryMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  /// distance from the boundary (centroid of boundary element to centroid of marked element)
  const Real _distance;

  /// lists of boundary elements for all boundaries
  const std::unordered_map<boundary_id_type, std::unordered_set<dof_id_type>> & _bnd_elem_ids;

  /// which way to mark elements near the boundary
  const MarkerValue _mark;

  /// boundary near which to mark elements
  const BoundaryID _boundary;
};
