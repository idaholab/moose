//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MESHMODIFIER_H
#define MESHMODIFIER_H

#include "MooseObject.h"
#include "Restartable.h"

// Forward declarations
class MeshModifier;
class MooseMesh;

template <>
InputParameters validParams<MeshModifier>();

/**
 * MeshModifiers are objects that can modify or add to an existing mesh.
 */
class MeshModifier : public MooseObject, public Restartable
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  MeshModifier(const InputParameters & parameters);

  /**
   * The base method called to trigger modification to the Mesh.
   * This method can trigger (re-)initialiation of the Mesh if
   * necessary, modify the mesh through the virtual override, and
   * also force prepare the mesh if requested.
   */
  void modifyMesh(MooseMesh * mesh, MooseMesh * displaced_mesh);

  /**
   * Return the MeshModifiers that must run before this MeshModifier
   */
  std::vector<std::string> & getDependencies() { return _depends_on; }

protected:
  /**
   * This method is called _immediatly_ before modify to perform any necessary
   * initialization on the modififer before it runs.
   */
  virtual void initialize() {}

  /**
   * Pure virtual modify function MUST be overridden by children classes.
   * This is where the MeshModifier actually does it's work.
   */
  virtual void modify() = 0;

  /**
   * Utility for performing the same operation on both undiplaced and
   * displaced meshes.
   */
  void modifyMeshHelper(MooseMesh * mesh);

  /// Pointer to the mesh
  MooseMesh * _mesh_ptr;

private:
  /// A list of modifiers that are required to run before this modifier may run
  std::vector<std::string> _depends_on;

  /// Flag to determine if the mesh should be prepared after this modifier is run
  const bool _force_prepare;
};

#endif // MESHMODIFIER_H
