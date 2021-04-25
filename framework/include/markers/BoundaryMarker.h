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
 * Marks all elements with sides on a given boundary for refinement/coarsening
 */
class BoundaryMarker : public Marker
{
public:
  static InputParameters validParams();

  BoundaryMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  const Real _distance;
  const std::unordered_map<boundary_id_type, std::unordered_set<dof_id_type>> & _bnd_elem_ids;
  MarkerValue _mark;
  BoundaryID _boundary;
};
