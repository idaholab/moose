//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// App includes
#include "AutomaticMortarGeneration.h"
#include "RelationshipManager.h"

// libMesh includes
#include "libmesh/mesh_base.h"

using libMesh::boundary_id_type;
using libMesh::CouplingMatrix;
using libMesh::Elem;
using libMesh::GhostingFunctor;
using libMesh::MeshBase;
using libMesh::processor_id_type;

class GhostBoundary : public RelationshipManager
{
public:
  GhostBoundary(const InputParameters &);

  GhostBoundary(const GhostBoundary & others);

  static InputParameters validParams();

  /**
   * This function must be overriden by application codes to add
   * required elements from (range_begin, range_end) to the
   * coupled_elements map.
   */
  virtual void operator()(const MeshBase::const_element_iterator & /*range_begin*/,
                          const MeshBase::const_element_iterator & /*range_end*/,
                          processor_id_type p,
                          map_type & coupled_elements) override;

  virtual std::unique_ptr<GhostingFunctor> clone() const override;

  virtual void redistribute() override { this->mesh_reinit(); }

  std::string getInfo() const override;

  virtual bool operator>=(const RelationshipManager & other) const override;

protected:
  virtual void internalInitWithMesh(const MeshBase &) override;

  const std::vector<BoundaryName> _boundary_name;

  /// Whether this relationship manager is called when coupling functors are called when building
  /// the matrix sparsity pattern
  const bool _is_coupling_functor;

  /// Whether to ghost point neighbors of secondary lower subdomain elements and their
  /// cross mortar interface counterparts for applications such as mortar nodal auxiliary kernels
  const bool _ghost_point_neighbors;

  /// null matrix for generating full variable coupling
  const CouplingMatrix * const _null_mat = nullptr;
};
