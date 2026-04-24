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
 * Reads an mfem::ParMesh from a file. All shared MFEM mesh plumbing lives in
 * MFEMMesh; this class only adds the file parameter and implements
 * buildSerialMFEMMesh() to load the mesh from disk.
 */
class MFEMFileMesh : public MFEMMesh
{
public:
  static InputParameters validParams();

  MFEMFileMesh(const InputParameters & parameters);
  virtual ~MFEMFileMesh();

  std::unique_ptr<MooseMesh> safeClone() const override;

protected:
  mfem::Mesh buildSerialMFEMMesh() override;
};

#endif
