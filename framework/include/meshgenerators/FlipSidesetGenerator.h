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
 * MeshGenerator for flipping a sideset
 */
class FlipSidesetGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  FlipSidesetGenerator(const InputParameters & parameters);

protected:
  std::unique_ptr<MeshBase> generate() override;

private:
  /// Input mesh the operation will be applied to
  std::unique_ptr<MeshBase> & _input;

  /// Name of the sideset to flip
  const BoundaryName _sideset_name;
};
