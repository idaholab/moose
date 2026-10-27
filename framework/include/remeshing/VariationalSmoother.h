//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LaplaceSmoother.h"

/**
 * Moves the mesh between remesh events with the LaplaceSmoother's harmonic solve as a predictor
 * and libMesh's VariationalMeshSmoother as a corrector, which relocates the nodes to minimize the
 * Branets-Carey distortion-dilation metric of the elements.
 *
 * The harmonic predictor is what keeps the correction well posed: it moves the interior and the
 * sliding walls together with the prescribed boundaries, so the configuration handed to the
 * variational solve is untangled even when a moving boundary travels along a wall whose nodes it
 * would otherwise overtake. The corrector then rearranges the nodes for element quality, with
 * every boundary node constrained from the current geometry - a node on a flat stretch of
 * boundary slides along it and a corner is pinned where it sits - so the boundary keeps the shape
 * the predictor prescribed.
 */
class VariationalSmoother : public LaplaceSmoother
{
public:
  static InputParameters validParams();

  VariationalSmoother(const InputParameters & parameters);

  virtual void updatePseudoDisplacement(Real dt) override;

private:
  /// The weight of the dilation metric relative to the distortion metric in the corrector
  const Real _dilation_weight;
};
