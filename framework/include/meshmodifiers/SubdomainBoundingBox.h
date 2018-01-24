//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SUBDOMAINBOUNDINGBOX_H
#define SUBDOMAINBOUNDINGBOX_H

// MOOSE includes
#include "MooseEnum.h"
#include "MeshModifier.h"

// Forward declerations
class SubdomainBoundingBox;

template <>
InputParameters validParams<SubdomainBoundingBox>();

namespace libMesh
{
class BoundingBox;
}

/**
 * MeshModifier for defining a Subdomain inside or outside of a bounding box
 */
class SubdomainBoundingBox : public MeshModifier
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters
   */
  SubdomainBoundingBox(const InputParameters & parameters);

  virtual void modify() override;

private:
  /// ID location (inside of outside of box)
  MooseEnum _location;

  /// Block ID to assign to the region
  SubdomainID _block_id;

  /// Bounding box for testing element centroids against
  BoundingBox _bounding_box;
};

#endif // SUBDOMAINBOUDINGBOX_H
