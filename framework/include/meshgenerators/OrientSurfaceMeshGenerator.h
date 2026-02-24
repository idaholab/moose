//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SubdomainsGeneratorBase.h"

/**
 * This class will re-orient surface elements based on user-specified settings
 */
class OrientSurfaceMeshGenerator : public SubdomainsGeneratorBase
{
public:
  static InputParameters validParams();

  OrientSurfaceMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;
};
