//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "MooseApp.h"

/**
 * Mesh generated from parameters
 */
class MeshGeneratorMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  MeshGeneratorMesh(const InputParameters & parameters);
  MeshGeneratorMesh(const MeshGeneratorMesh & /* other_mesh */) = default;

  // No copy
  MeshGeneratorMesh & operator=(const MeshGeneratorMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;
};
