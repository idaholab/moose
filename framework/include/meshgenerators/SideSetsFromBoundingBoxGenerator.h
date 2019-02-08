//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SIDESETSFROMBOUNDINGBOXGENERATOR_H
#define SIDESETSFROMBOUNDINGBOXGENERATOR_H

#include "MeshGenerator.h"
#include "MooseEnum.h"
#include "libmesh/bounding_box.h"

// Forward declarations
class SideSetsFromBoundingBoxGenerator;

template <>
InputParameters validParams<SideSetsFromBoundingBoxGenerator>();

/**
 * MeshGenerator for defining a Subdomain inside or outside of a bounding box
 */
class SideSetsFromBoundingBoxGenerator : public MeshGenerator
{
public:
  SideSetsFromBoundingBoxGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate();

protected:
  std::unique_ptr<MeshBase> & _input;

  /// ID location (inside of outside of box)
  MooseEnum _location;

  /// Block ID to assign to the region
  subdomain_id_type _block_id;

  /// boundary ID to select
  std::vector<BoundaryName> _boundary_id_old;

  /// boundary ID to assign
  boundary_id_type _boundary_id_new;

  /// Bounding box for testing element centroids against
  BoundingBox _bounding_box;

  /// Flag to determine if the provided boundaries need to overlap
  const bool _boundary_id_overlap;
};

#endif // SIDESETSFROMBOUNDINGBOXGENERATOR_H
