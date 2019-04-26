//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideSetsGeneratorBase.h"

// Forward declarations
class SideSetsFromNormalsGenerator;

template <>
InputParameters validParams<SideSetsFromNormalsGenerator>();

/**
 *
 */
class SideSetsFromNormalsGenerator : public SideSetsGeneratorBase
{
public:
  SideSetsFromNormalsGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  std::vector<BoundaryName> _boundary_names;

  std::vector<Point> _normals;
};

