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

class AugmentSparsityOnInterface : public RelationshipManager
{
public:
  AugmentSparsityOnInterface(const InputParameters &);

  AugmentSparsityOnInterface(const AugmentSparsityOnInterface & others);

  static InputParameters validParams();

  /**
   * This function must be overriden by application codes to add
   * required elements from (range_begin, range_end) to the
   * coupled_elements map.
   */
  virtual void operator()(const MeshBase::const_element_iterator & range_begin,
                          const MeshBase::const_element_iterator & range_end,
                          processor_id_type p,
                          map_type & coupled_elements) override;

  /**
   * A clone() is needed because GhostingFunctor can not be shared between
   * different meshes. The operations in  GhostingFunctor are mesh dependent.
   */
  virtual std::unique_ptr<GhostingFunctor> clone() const override
  {
    return std::make_unique<AugmentSparsityOnInterface>(*this);
  }

  /**
   * Update the cached _lower_to_upper map whenever our Mesh has been
   * redistributed.  We'll be lazy and just recalculate from scratch.
   */
  virtual void redistribute() override { this->mesh_reinit(); }

  std::string getInfo() const override;

  virtual bool operator>=(const RelationshipManager & other) const override;

protected:
  virtual void internalInitWithMesh(const MeshBase &) override;

  /**
   * Ghost the mortar interface couplings of the provided element
   */
  void ghostMortarInterfaceCouplings(const processor_id_type p,
                                     const Elem * const elem,
                                     map_type & coupled_elements,
                                     const AutomaticMortarGeneration & amg) const;

  /**
   * Query the mortar interface couplings of the query element. If a lower dimensional secondary
   * element is found, then we search for its point neighbors, which we ghost, as well as all of the
   * mortar interface couplings of the point neighbors. This kind of ghosting is required for mortar
   * nodal auxiliary kernels
   */
  void ghostLowerDSecondaryElemPointNeighbors(const processor_id_type p,
                                              const Elem * const query_elem,
                                              map_type & coupled_elements,
                                              BoundaryID secondary_boundary_id,
                                              SubdomainID secondary_subdomain_id,
                                              const AutomaticMortarGeneration & amg) const;

  void ghostHigherDNeighbors(const processor_id_type p,
                             const Elem * const query_elem,
                             map_type & coupled_elements,
                             BoundaryID secondary_boundary_id,
                             SubdomainID secondary_subdomain_id,
                             const AutomaticMortarGeneration & amg) const;

  const BoundaryName _primary_boundary_name;
  const BoundaryName _secondary_boundary_name;
  const SubdomainName _primary_subdomain_name;
  const SubdomainName _secondary_subdomain_name;

  /// Whether this relationship manager is called when coupling functors are called when building
  /// the matrix sparsity pattern
  const bool _is_coupling_functor;

  /// Whether to ghost point neighbors of secondary lower subdomain elements and their
  /// cross mortar interface counterparts for applications such as mortar nodal auxiliary kernels
  const bool _ghost_point_neighbors;

  /// Whether we should ghost higher-dimensional neighbors. This is necessary when we are doing
  /// second order mortar with finite volume primal variables, because in order for the method to be
  /// second order we must use cell gradients, which couples in the neighbor cells
  const bool _ghost_higher_d_neighbors;

  /// null matrix for generating full variable coupling
  const CouplingMatrix * const _null_mat = nullptr;
};
