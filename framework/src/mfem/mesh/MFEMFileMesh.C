//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMFileMesh.h"

registerMooseObject("MooseApp", MFEMFileMesh);

InputParameters
MFEMFileMesh::validParams()
{
  InputParameters params = MFEMMesh::validParams();
  params.addRequiredParam<MeshFileName>("file", "The name of the mesh file to read.");
  params.addClassDescription("Reads an mfem::ParMesh from a file.");
  return params;
}

MFEMFileMesh::MFEMFileMesh(const InputParameters & parameters) : MFEMMesh(parameters) {}

MFEMFileMesh::~MFEMFileMesh() {}

mfem::Mesh
MFEMFileMesh::buildSerialMFEMMesh()
{
  buildDummyMooseMesh();
  return mfem::Mesh(getParam<MeshFileName>("file").c_str());
}

std::unique_ptr<MooseMesh>
MFEMFileMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}

#endif
