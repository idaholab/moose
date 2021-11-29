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
 * Creates lower-dimensional elements on the specified sidesets
 */
class LowerDBlockFromSidesetGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  LowerDBlockFromSidesetGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;
  /// a vector of the names of the sidesets to add the lower-D elements to
  const std::vector<BoundaryName> _sideset_names;
};
