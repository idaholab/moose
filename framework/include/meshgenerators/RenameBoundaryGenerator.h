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
 * MeshGenerator for re-numbering or re-naming boundaries
 */
class RenameBoundaryGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  RenameBoundaryGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  /// The old boundaries
  std::vector<BoundaryName> _old_boundary;
  /// The new boundaries
  std::vector<BoundaryName> _new_boundary;
  /// The name of the parameter that specifies the old boundaries
  std::string _old_boundary_param_name;
  /// The name of the parameter that specifies the new boundaries
  std::string _new_boundary_param_name;
};
