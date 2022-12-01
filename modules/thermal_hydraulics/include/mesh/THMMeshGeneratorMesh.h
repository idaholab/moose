//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGeneratorMesh.h"

/**
 * Mesh generated using mesh generators for the thermal hydraulics module.
 */
class THMMeshGeneratorMesh : public MeshGeneratorMesh
{
public:
  static InputParameters validParams();

  THMMeshGeneratorMesh(const InputParameters & parameters);
  THMMeshGeneratorMesh(const THMMeshGeneratorMesh & other_mesh) = default;

  THMMeshGeneratorMesh & operator=(const THMMeshGeneratorMesh & other_mesh) = delete;

  virtual unsigned int dimension() const override;
  virtual unsigned int effectiveSpatialDimension() const override;
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
};
