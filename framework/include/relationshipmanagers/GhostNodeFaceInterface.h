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

// libMesh includes
#include "libmesh/mesh_base.h"

using libMesh::CouplingMatrix;
using libMesh::Elem;
using libMesh::GhostingFunctor;
using libMesh::MeshBase;
using libMesh::processor_id_type;

/**
 * Ghosts the higher-dimensional elements on a node-face constraint's primary and secondary
 * boundaries to processors that own elements on the secondary boundary.
 */
class GhostNodeFaceInterface : public RelationshipManager
{
public:
  GhostNodeFaceInterface(const InputParameters & params);

  GhostNodeFaceInterface(const GhostNodeFaceInterface & other);

  static InputParameters validParams();

  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;

  virtual std::unique_ptr<GhostingFunctor> clone() const override;

  virtual void redistribute() override { this->mesh_reinit(); }

  std::string getInfo() const override;

  virtual bool operator>=(const RelationshipManager & other) const override;

protected:
  virtual void internalInitWithMesh(const MeshBase &) override;

  bool hasSecondaryBoundaryFace(const MeshBase::const_element_iterator & range_begin,
                                const MeshBase::const_element_iterator & range_end,
                                BoundaryID secondary_boundary_id,
                                bool generating_mesh) const;

  const BoundaryName _primary_boundary_name;
  const BoundaryName _secondary_boundary_name;
  const bool _enabled;

  /// null matrix for generating full variable coupling
  const CouplingMatrix * const _null_mat = nullptr;
};
