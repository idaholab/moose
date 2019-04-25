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

// Forward declarations
class LowerDBlockFromSidesetGenerator;

template <>
InputParameters validParams<LowerDBlockFromSidesetGenerator>();

/**
 * Creates lower-dimensional elements on the specified sidesets
 */
class LowerDBlockFromSidesetGenerator : public MeshGenerator
{
public:
  LowerDBlockFromSidesetGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  /// The subdomain ID of the new lower dimensional block
  const subdomain_id_type _new_block_id;
  /// The sidesets on which to create the lower dimensional elements
  std::vector<boundary_id_type> _sidesets;
};

