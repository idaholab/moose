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
 * Generates a mesh by reading it from an file.
 */
class FileMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  FileMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// the path/name of the file containing the mesh
  const MeshFileName & _file_name;

  /// whether to skip partitioning after loading the mesh
  const bool _skip_partitioning;

  /// Whether to allow renumbering (for non-exodus files) when the mesh is read and prepared for use
  const bool _allow_renumbering;
};
