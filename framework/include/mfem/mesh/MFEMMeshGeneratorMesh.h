//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMMesh.h"

/**
 * MooseMesh subclass that consumes the MeshGenerator pipeline to build an
 * mfem::ParMesh. It is the MFEM analog of MeshGeneratorMesh: use it as the
 * [Mesh] type (or let SetupMeshAction auto-select it) whenever one or more
 * MFEMMeshGenerator objects are present.
 *
 * All shared MFEM mesh plumbing lives in MFEMMesh; this class implements
 * buildSerialMFEMMesh() to extract the mfem::Mesh from the MFEMMeshCarrier
 * produced by the generator pipeline.
 */
class MFEMMeshGeneratorMesh : public MFEMMesh
{
public:
  static InputParameters validParams();

  MFEMMeshGeneratorMesh(const InputParameters & parameters);

  std::unique_ptr<MooseMesh> safeClone() const override;

protected:
  mfem::Mesh buildSerialMFEMMesh() override;
};

#endif
