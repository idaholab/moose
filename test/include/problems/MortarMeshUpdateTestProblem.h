//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "FEProblem.h"

/**
 * Test problem that verifies displaced mortar meshes are rebuilt during Jacobian assembly.
 */
class MortarMeshUpdateTestProblem : public FEProblem
{
public:
  static InputParameters validParams();

  MortarMeshUpdateTestProblem(const InputParameters & params);

  void computeJacobianTags(const std::set<TagID> & tags) override;
  void updateMortarMesh() override;

private:
  bool _updated_mortar_mesh = false;
};
