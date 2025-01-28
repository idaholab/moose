//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "CutMeshByLevelSetGeneratorBase.h"

/**
 * This CutMeshByLevelSetGenerator object is designed to trim the input mesh by removing all the
 * elements on outside the give level set with special processing on the elements crossed by the
 * cutting surface to ensure a smooth cross-section. The output mesh only consists of TET4
 * elements.
 */
class CutMeshByLevelSetGenerator : public CutMeshByLevelSetGeneratorBase
{
public:
  static InputParameters validParams();

  CutMeshByLevelSetGenerator(const InputParameters & parameters);

protected:
  /// The analytic level set function in the form of a string that can be parsed by FParser
  const std::string _level_set;
};
