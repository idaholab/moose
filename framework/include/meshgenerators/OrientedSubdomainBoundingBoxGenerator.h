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
#include "OrientedBoxInterface.h"

/**
 * MeshGenerator for defining a Subdomain inside or outside of a bounding box with arbitrary
 * orientation
 */
class OrientedSubdomainBoundingBoxGenerator : public MeshGenerator, public OrientedBoxInterface
{
public:
  static InputParameters validParams();

  OrientedSubdomainBoundingBoxGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  /// ID location (inside or outside of the bounding box)
  const MooseEnum _location;

  /// Block ID to assign to the region
  const subdomain_id_type _block_id;
};
