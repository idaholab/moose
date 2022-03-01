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
 * MeshGenerator for defining a Subdomain inside or outside of a bounding box
 */
class SubdomainBoundingBoxGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SubdomainBoundingBoxGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  /// ID location (inside of outside of box)
  MooseEnum _location;

  /// Block ID to assign to the region
  subdomain_id_type _block_id;

  /// Whether or not we apply the bounding box only for certain subdomains
  const bool _has_restriction;

  /// Bounding box for testing element centroids against
  BoundingBox _bounding_box;
};
