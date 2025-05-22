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

/**
 * MeshGenerator for defining a subdomain on the layer next to one or more boundaries
 */
class BoundaryLayerSubdomainGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  BoundaryLayerSubdomainGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  /// Block ID to assign to the boundary layer region
  subdomain_id_type _new_block_id;
  /// Block name to assign to the boundary layer region
  const SubdomainName _new_block_name;
  /// Whether to consider nodesets in the boundary proximity check
  const bool _include_nodesets;
};
