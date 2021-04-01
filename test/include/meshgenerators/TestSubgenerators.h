//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StitchedMeshGenerator.h"

/**
 * Allows multiple mesh files to be "stitched" together to form a single mesh.
 */
class TestSubgenerators : public StitchedMeshGenerator
{
public:
  static InputParameters validParams();

  TestSubgenerators(const InputParameters & parameters);

protected:
  /// The mesh generator input filenames to read
  const std::vector<std::string> & _input_filenames;
};
