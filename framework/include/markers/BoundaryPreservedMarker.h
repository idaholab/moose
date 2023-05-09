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

/*
  Do not coarsen any element touching a given boundary in order to preserve the boundary
  libMesh will delete all boundaries for the elements to be coarsened.
*/
class BoundaryPreservedMarker : public Marker
{
public:
  static InputParameters validParams();
  BoundaryPreservedMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  // Check whether or not to coarsen the current element
  // It is fine to coarsen the current element if it does not touch the given boundary
  // Otherwise return false
  bool preserveBoundary(const Elem * const & _current_elem);

  // Boundary to be preserved during coarsening (AMR)
  BoundaryID _preserved_boundary;

  MarkerName _marker_name;

  const VariableValue * _marker;
};
