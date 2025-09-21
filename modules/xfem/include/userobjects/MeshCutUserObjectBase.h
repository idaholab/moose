//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeometricCutUserObject.h"
/**
 * Simple base class for XFEM cutting objects that use a mesh to cut.
 * This is mainly for output of the cutting mesh to exodus.
 */

class MeshCutUserObjectBase : public GeometricCutUserObject
{
public:
  static InputParameters validParams();

  MeshCutUserObjectBase(const InputParameters & parameters);
  /**
   * Get a reference to the cutter mesh
   * @return reference to the cutter mesh
   */
  MeshBase & getCutterMesh() const;

protected:
  /// The xfem cutter mesh
  std::unique_ptr<MeshBase> _cutter_mesh;
};
