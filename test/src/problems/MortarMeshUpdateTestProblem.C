//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "MortarMeshUpdateTestProblem.h"

registerMooseObject("MooseTestApp", MortarMeshUpdateTestProblem);

InputParameters
MortarMeshUpdateTestProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addClassDescription(
      "Verifies that displaced mortar meshes are rebuilt during Jacobian assembly.");
  return params;
}

MortarMeshUpdateTestProblem::MortarMeshUpdateTestProblem(const InputParameters & params)
  : FEProblem(params)
{
}

void
MortarMeshUpdateTestProblem::computeJacobianTags(const std::set<TagID> & tags)
{
  _updated_mortar_mesh = false;
  FEProblem::computeJacobianTags(tags);

  if (!_updated_mortar_mesh)
    mooseError("The displaced mortar mesh was not rebuilt during Jacobian assembly.");
}

void
MortarMeshUpdateTestProblem::updateMortarMesh()
{
  FEProblem::updateMortarMesh();
  _updated_mortar_mesh = true;
}
