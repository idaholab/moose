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

/**
 * Removes the refinement tree of an h-refined input mesh, keeping only the
 * finest (active) elements as unrefined level-0 elements. Optionally converts
 * the flattened mesh to first order.
 */
class FlattenRefinementGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  FlattenRefinementGenerator(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  /// The input mesh whose refinement will be flattened
  std::unique_ptr<MeshBase> & _input;

  /// Whether to also convert the flattened mesh to first order
  const bool _first_order;
};
