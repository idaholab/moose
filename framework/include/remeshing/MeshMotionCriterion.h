//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RemeshCriterion.h"

/**
 * Fires when the mesh motion accumulated since the last remesh becomes large compared to the local
 * element size.
 *
 * The measured quantity is the maximum over the active elements of the largest nodal
 * pseudo-displacement of the element divided by its diameter, which is reduced over the
 * communicator before it is compared to the threshold.
 *
 * The pseudo-displacement is identically zero unless the [Remeshing] block sets
 * mesh_movement = true, so this criterion requires it.
 */
class MeshMotionCriterion : public RemeshCriterion
{
public:
  static InputParameters validParams();

  MeshMotionCriterion(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual bool shouldRemesh() override;

private:
  /// The pseudo-displacement to element size ratio that no active element is allowed to exceed
  const Real _threshold;
};
