//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMBoundarySubMesh.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMBoundarySubMesh);

InputParameters
MFEMBoundarySubMesh::validParams()
{
  InputParameters params = MFEMSubMesh::validParams();
  params += MFEMBoundaryRestrictable::validParams();

  params.addClassDescription("Class to construct an MFEMSubMesh formed from the subspace of the "
                             "parent mesh restricted to the set of user-specified boundaries.");
  return params;
}

MFEMBoundarySubMesh::MFEMBoundarySubMesh(const InputParameters & parameters)
  : MFEMSubMesh(parameters),
    MFEMBoundaryRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh())
{
}

void
MFEMBoundarySubMesh::buildSubMesh()
{
  _submesh = std::make_shared<mfem::ParSubMesh>(
      mfem::ParSubMesh::CreateFromBoundary(getMesh(), getBoundaryAttributes()));
}

#endif
