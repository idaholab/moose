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

#include "libmesh/replicated_mesh.h"

/**
 * A thin libMesh::ReplicatedMesh subclass that carries an mfem::Mesh through the
 * MeshGenerator pipeline. The libMesh side is always empty; it exists only to satisfy
 * the std::unique_ptr<MeshBase> return type of MeshGenerator::generate(). MFEM mesh
 * generators wrap their output in this carrier and unwrap their inputs from it.
 */
class MFEMMeshCarrier : public libMesh::ReplicatedMesh
{
public:
  explicit MFEMMeshCarrier(const Parallel::Communicator & comm);

  void setMFEMMesh(mfem::Mesh mesh) { _mfem_mesh = std::move(mesh); }

  mfem::Mesh & getMFEMMesh() { return _mfem_mesh; }
  const mfem::Mesh & getMFEMMesh() const { return _mfem_mesh; }

  /// Move the held mfem::Mesh out; leaves the carrier with a default-constructed (empty) mesh.
  mfem::Mesh releaseMFEMMesh() { return std::move(_mfem_mesh); }

private:
  mfem::Mesh _mfem_mesh;
};

#endif
