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
 * MeshGenerator that checks if a set of blocks is enclosed by a set of sidesets.
 * It can either add sides that are not covered by a sideset by a new sidesets or
 * error out.
 */
class SidesetsEncloseBlocks : public MeshGenerator
{
public:
  static InputParameters validParams();

  SidesetsEncloseBlocks(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// the mesh that is passed from the meshgen executed before this meshgen
  std::unique_ptr<MeshBase> & _input;
};
