//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GENERATEBOUNDINGBOXNODESET_H
#define GENERATEBOUNDINGBOXNODESET_H

#include "MeshGenerator.h"
#include "MooseEnum.h"

// Forward declarations
class GenerateBoundingBoxNodeSet;

namespace libMesh
{
class BoundingBox;
}

template <>
InputParameters validParams<GenerateBoundingBoxNodeSet>();

/**
 * Selects a set of nodes and assigns a nodeset name to them based on
 * the bounding box specified. Can select nodes "inside" or "outside"
 * the bounding box.
 */
class GenerateBoundingBoxNodeSet : public MeshGenerator
{
public:
  GenerateBoundingBoxNodeSet(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate();

protected:
  std::unique_ptr<MeshBase> & _input;

  /// Select nodes on the 'inside' or the 'outside' of the bounding box
  MooseEnum _location;

  /// Bounding box for testing element centroids against. Note that
  BoundingBox _bounding_box;
};

#endif // GENERATEBOUNDINGBOXNODESET_H
