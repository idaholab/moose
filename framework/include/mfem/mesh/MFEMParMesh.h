#pragma once

#include "mfem.hpp"
#include "MFEMMesh.h"

/**
 * MFEMParMesh
 *
 * MFEMParMesh is a wrapper for the mfem::ParMesh object.
 */
class MFEMParMesh : public mfem::ParMesh
{
public:
  MFEMParMesh(MPI_Comm comm, MFEMMesh & mesh, int * partitioning = nullptr, int part_method = 1)
    : mfem::ParMesh(comm, dynamic_cast<mfem::Mesh &>(mesh), partitioning, part_method){};

  // TODO: - add implementation.
};
