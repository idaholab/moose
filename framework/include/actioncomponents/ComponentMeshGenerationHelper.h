//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActionComponent.h"
#include "InputParameters.h"
#include "MooseTypes.h"

/**
 * Helper class to help Components be located with the orientation and position we want.
 * The template argument is the dimension: 0D components don't need to be re-oriented,
 * while 1D, 2D and 3D components could need this feature.
 */
class ComponentMeshGenerationHelper : public virtual ActionComponent
{
public:
  static InputParameters validParams();

  ComponentMeshGenerationHelper(const InputParameters & params);

protected:
  /**
   * Adds a mesh generator to the MeshGeneratorSystem
   * @param type type of the mesh generator to create
   * @param suffix suffix to append to the name of the component
   * @param params parameters to create the mesh generator with
   * The verbosity parameters will be set in the params (hence the non-const reference)
   */
  void
  addMeshGenerator(const std::string & type, const std::string & suffix, InputParameters & params);

  // protected and not private in case addMeshGenerator is bypassed
  /// Whether to show the current status of the mesh in the console after each mesh generation step
  const bool _show_mesh_generation_info;
  /// Whether to show the output the mesh to exodus after each mesh generation step
  const bool _output_intermediate_meshes;
  /// Whether to introduce a number in the mesh generator name to facilitate sorting
  const bool _number_mesh_generator_output;
};
