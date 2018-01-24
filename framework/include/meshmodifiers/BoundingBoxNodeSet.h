//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BOUNDINGBOXNODESET_H
#define BOUNDINGBOXNODESET_H

// MOOSE includes
#include "MeshModifier.h"
#include "MooseEnum.h"

// Forward Declaration
class BoundingBoxNodeSet;
namespace libMesh
{
class BoundingBox;
}

template <>
InputParameters validParams<BoundingBoxNodeSet>();

/**
 * Selects a set of nodes and assigns a nodeset name to them based on
 * the bounding box specified. Can select nodes "inside" or "outside"
 * the bounding box.
 */
class BoundingBoxNodeSet : public MeshModifier
{
public:
  BoundingBoxNodeSet(const InputParameters & parameters);

protected:
  virtual void modify() override;

private:
  /// Select nodes on the 'inside' or the 'outside' of the bounding box
  MooseEnum _location;

  /// Bounding box for testing element centroids against. Note that
  BoundingBox _bounding_box;
};

#endif // BOUNDINGBOXNODESET_H
