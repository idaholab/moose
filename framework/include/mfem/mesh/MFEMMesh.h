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
 * MFEMMeshGeneratorMesh). Holds the mfem::ParMesh and provides common
 * operations (refinement, reordering, partitioning, displacement, recovery).
 * Subclasses implement buildSerialMFEMMesh() to supply the serial mesh;
 * buildMesh() applies the common operations using the template-method pattern.
 */
class MFEMMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  MFEMMesh(const InputParameters & parameters);

  /**
   * Accessors for the _mfem_par_mesh object. If the mesh has
   * not been built, the methods will call the appropriate protected methods to
   * build them.
   */
  mfem::ParMesh & getMFEMParMesh() { return *_mfem_par_mesh; }
  const mfem::ParMesh & getMFEMParMesh() const;

  /**
   * Copy a shared_ptr to the mfem::ParMesh object.
   */
  std::shared_ptr<mfem::ParMesh> getMFEMParMeshPtr() { return _mfem_par_mesh; }

  /**
   * Initialize the MFEM ParMesh and placeholder MOOSE mesh.
   */
  void init() override;
  std::vector<std::filesystem::path>
  writeRecoveryFiles(const std::filesystem::path & file_base) override;

  /**
   * Returns true if mesh displacement is required.
   */
  bool shouldDisplace() const { return _mesh_displacement_variable.has_value(); }

  /**
   * Returns an optional reference to displacement variable name.
   */
  std::optional<std::reference_wrapper<std::string const>> getMeshDisplacementVariable() const
  {
    return _mesh_displacement_variable;
  }

  /**
   * Displace the nodes of the mesh by the given displacement.
   * Does not update FE spaces for variables.
   */
  void displace(mfem::GridFunction const & displacement);

  bool isDistributedMesh() const override { return true; }
  unsigned int dimension() const override { return _mfem_par_mesh->Dimension(); }
  unsigned int spatialDimension() const override { return _mfem_par_mesh->SpaceDimension(); }
  SubdomainID nSubdomains() const override { return _mfem_par_mesh->attributes.Size(); }
  dof_id_type nActiveElem() const override { return _mfem_par_mesh->GetGlobalNE(); }
  dof_id_type nActiveLocalElem() const override { return _mfem_par_mesh->GetNE(); }

  void buildMesh() override final;

protected:
  /**
   * Build and return the serial mfem::Mesh for this object.
   */
  virtual mfem::Mesh buildSerialMFEMMesh() = 0;

  /**
   * Builds a dimension-compatible libMesh placeholder after initializing the MFEM mesh.
   */
  void buildDummyMooseMesh();

  /**
   * Performs a uniform refinement on the chosen mesh nref times.
   */
  void uniformRefinement(mfem::Mesh & mesh, const unsigned int nref) const;

  /**
   * Holds name of variable used for mesh displacement, if set.
   */
  std::optional<std::string> _mesh_displacement_variable;

  /**
   * Smart pointer to mfem::ParMesh object. Do not access directly.
   * Use the accessors instead.
   */
  std::shared_ptr<mfem::ParMesh> _mfem_par_mesh{nullptr};
};

inline const mfem::ParMesh &
MFEMMesh::getMFEMParMesh() const
{
  return const_cast<MFEMMesh *>(this)->getMFEMParMesh();
}

#endif
