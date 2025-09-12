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
 * Simple base class holding cutter mesh to give to outputs block.
 * TODO:  move more common 2D and 3D features into this class
 */

class MeshCutUserObjectBase : public GeometricCutUserObject
{
public:
  static InputParameters validParams();

  MeshCutUserObjectBase(const InputParameters & parameters);

  MeshBase & getCutterMesh() const;

protected:
  /// The xfem cutter mesh
  std::unique_ptr<MeshBase> _cutter_mesh;
};
