//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RelationshipManager.h"
#include "libmesh/mesh_base.h"

/**
 * Ghosts full-dimensional elements with sides on the primary boundary to processors that own
 * full-dimensional elements with sides on the secondary boundary.
 */
class GhostPrimaryFace : public RelationshipManager
{
public:
  GhostPrimaryFace(const InputParameters & params);

  GhostPrimaryFace(const GhostPrimaryFace & other);

  static InputParameters validParams();

  virtual void operator()(const libMesh::MeshBase::const_element_iterator & range_begin,
                          const libMesh::MeshBase::const_element_iterator & range_end,
                          libMesh::processor_id_type p,
                          map_type & coupled_elements) override;

  virtual std::unique_ptr<libMesh::GhostingFunctor> clone() const override;

  virtual void redistribute() override { this->mesh_reinit(); }

  std::string getInfo() const override;

  virtual bool operator>=(const RelationshipManager & other) const override;

protected:
  virtual void internalInitWithMesh(const libMesh::MeshBase &) override;

  /// Return whether the local element range contains a secondary boundary face.
  bool hasSecondaryBoundaryFace(const libMesh::MeshBase::const_element_iterator & range_begin,
                                const libMesh::MeshBase::const_element_iterator & range_end,
                                BoundaryID secondary_boundary_id,
                                bool generating_mesh) const;

  /// Primary boundary whose full-dimensional elements may be ghosted.
  const BoundaryName _primary_boundary_name;

  /// Secondary boundary whose local presence triggers primary boundary ghosting.
  const BoundaryName _secondary_boundary_name;

  /// Whether this relationship manager should add ghosting entries.
  const bool _enabled;

  /// null matrix for generating full variable coupling
  const libMesh::CouplingMatrix * const _null_mat = nullptr;
};
