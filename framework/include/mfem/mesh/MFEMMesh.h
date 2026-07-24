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

#include "MooseMesh.h"

/**
 * Abstract MooseMesh base for all MFEM-backed mesh types (MFEMFileMesh,
 * MFEMMeshGeneratorMesh). Holds the mfem::ParMesh and owns the shared
 * MFEM mesh interface. Subclasses implement buildSerialMFEMMesh() to
 * supply the serial mesh; buildMesh() handles the common post-processing
 * (refinement, reordering, partitioning) using the template-method pattern.
 */
class MFEMMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  MFEMMesh(const InputParameters & parameters);

  mfem::ParMesh & getMFEMParMesh() { return *_mfem_par_mesh; }
  const mfem::ParMesh & getMFEMParMesh() const;

  std::shared_ptr<mfem::ParMesh> getMFEMParMeshPtr() { return _mfem_par_mesh; }

  bool shouldDisplace() const { return _mesh_displacement_variable.has_value(); }

  std::optional<std::reference_wrapper<std::string const>> getMeshDisplacementVariable() const
  {
    return _mesh_displacement_variable;
  }

  void displace(mfem::GridFunction const & displacement);

  bool isDistributedMesh() const override { return true; }
  unsigned int dimension() const override { return _mfem_par_mesh->Dimension(); }
  unsigned int spatialDimension() const override { return _mfem_par_mesh->SpaceDimension(); }
  SubdomainID nSubdomains() const override { return _mfem_par_mesh->attributes.Size(); }
  dof_id_type nActiveElem() const override { return _mfem_par_mesh->GetGlobalNE(); }
  dof_id_type nActiveLocalElem() const override { return _mfem_par_mesh->GetNE(); }

  void buildMesh() override;

protected:
  /**
   * Build and return the serial mfem::Mesh for this object.
   */
  virtual mfem::Mesh buildSerialMFEMMesh() = 0;

  /// Builds a dimension-compatible libMesh placeholder after initializing the MFEM mesh.
  void buildDummyMooseMesh();

  /// Applies uniform refinement to mesh nref times.
  void uniformRefinement(mfem::Mesh & mesh, unsigned int nref) const;

  std::optional<std::string> _mesh_displacement_variable;
  std::shared_ptr<mfem::ParMesh> _mfem_par_mesh{nullptr};
};

inline const mfem::ParMesh &
MFEMMesh::getMFEMParMesh() const
{
  return const_cast<MFEMMesh *>(this)->getMFEMParMesh();
}

#endif
