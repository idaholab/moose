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

#include "libmesh/bounding_box.h"

namespace libMesh
{
class BoundingBox;
}

/**
 * Selects a set of nodes and assigns a nodeset name to them based on
 * the bounding box specified. Can select nodes "inside" or "outside"
 * the bounding box.
 */
class BoundingBoxNodeSetGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  BoundingBoxNodeSetGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  /// Select nodes on the 'inside' or the 'outside' of the bounding box
  MooseEnum _location;

  /// Bounding box for testing element centroids against. Note that
  BoundingBox _bounding_box;
};
