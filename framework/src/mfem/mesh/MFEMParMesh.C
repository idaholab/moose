#pragma once
#include "MFEMParMesh.h"

// This is a temporary initializer and will be replaced once this class has been
// completed.
MFEMParMesh::MFEMParMesh(MPI_Comm comm, MFEMMesh & mesh, int * partitioning, int part_method)
  : mfem::ParMesh(comm, dynamic_cast<mfem::Mesh &>(mesh), partitioning, part_method)
{
}
