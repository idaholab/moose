#ifdef MFEM_ENABLED

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "FileMesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

/**
 * MFEMMesh
 *
 * MFEMMesh inherits a MOOSE mesh class which allows us to work with
 * other MOOSE objects. It contains a pointer to the parallel MFEM mesh.
 */
class MFEMMesh : public FileMesh
{
public:
  static InputParameters validParams();

  MFEMMesh(const InputParameters & parameters);

  virtual ~MFEMMesh();

  /**
   * Accessors for the _mfem_par_mesh object. If the mesh has
   * not been build, the methods will call the appropriate protected methods to
   * build them.
   */
  mfem::ParMesh & getMFEMParMesh() { return *_mfem_par_mesh; }
  const mfem::ParMesh & getMFEMParMesh() const;

  /**
   * Copy a shared_ptr to the mfem::ParMesh object.
   */
  std::shared_ptr<mfem::ParMesh> getMFEMParMeshPtr() { return _mfem_par_mesh; }

  /**
   * Build MFEM ParMesh and a placeholder MOOSE mesh.
   */
  void buildMesh() override;

  /**
   * Clones the mesh.
   */
  std::unique_ptr<MooseMesh> safeClone() const override;

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

private:
  /**
   * Builds a placeholder mesh when no MOOSE mesh is required.
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
   * Smart pointers to mfem::ParMesh object. Do not access directly.
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
