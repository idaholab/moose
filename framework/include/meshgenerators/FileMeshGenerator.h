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

  /**
   * Helper for deducing a checkpoint file given the path.
   *
   * We pick one of the following:
   * - If the path just exists, use it
   * - If not, but a path with the new suffix exists instead (i.e.,
   *   /path/to/xxxx_mesh.cpa.gz was provided but /path/to/xxxx-mesh.cpa.gz
   *   exists), use that path and provide a param warning via \p object
   * - If not, but it is LATEST and we can find a latest checkpoint,
   *   use the latest checkpoint
   *
   * This is a static method because FileMesh needs the same thing. And
   * eventually FileMesh should be axed in favor of this.
   */
  static std::string deduceCheckpointPath(const MooseObject & object,
                                          const std::string & file_name);

protected:
  /// the path/name of the file containing the mesh
  const MeshFileName & _file_name;

  /// the path/name of any file containing a matrix of mesh constraints
  const MatrixFileName & _matrix_file_name;

  /// whether to skip partitioning after loading the mesh
  const bool _skip_partitioning;

  /// Whether to allow renumbering (for non-exodus files) when the mesh is read and prepared for use
  const bool _allow_renumbering;
};
