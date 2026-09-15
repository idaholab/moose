//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosArray.h"
#include "KokkosQpJacobianTensor.h"

namespace Moose::Kokkos
{

/**
 * The quadrature-point linearization of every kernel active on a system, held in reference-element
 * form.
 *
 * The cache is filled once per linearization from the fine level's quadrature rule, by summing the
 * contribution of each active kernel at each quadrature point, and is thereafter the only input the
 * operator action and the operator diagonal need: a consumer contracts the stored tensors with a
 * basis table and never re-enters the physics. Levels of a p-multigrid hierarchy that share the
 * quadrature rule differ only in that basis table, so one fill serves them all.
 *
 * Because the fill happens inside the ordinary Jacobian sweep, the cache is consistent with the
 * solution state that sweep linearized about. Validity is tracked explicitly so that a consumer
 * cannot silently contract a tensor left over from an earlier linearization.
 *
 * Tensors are indexed by (subdomain, variable) and, within that, by the subdomain-local flattened
 * quadrature point index used throughout the Kokkos assembly. Off-diagonal variable pairs and
 * boundary faces have no entries: an object contributing to either continues to supply its own
 * operator action.
 */
class QpJacobianCache
{
public:
  /**
   * Allocate the per-(subdomain, variable) outer structures and clear every coupling mask
   * @param n_subdomains The number of mesh subdomains
   * @param n_vars The number of system variables
   */
  void create(unsigned int n_subdomains, unsigned int n_vars);

  /**
   * Allocate the quadrature-point tensors of a (subdomain, variable) pair, if not already allocated
   * @param subdomain The contiguous subdomain ID
   * @param var The variable number
   * @param n_qps The number of quadrature points in the subdomain
   */
  void createTensors(unsigned int subdomain, unsigned int var, dof_id_type n_qps);

  /**
   * Check whether the quadrature-point tensors of a (subdomain, variable) pair are allocated
   * @param subdomain The contiguous subdomain ID
   * @param var The variable number
   * @returns Whether the tensors are allocated
   */
  bool isAlloc(unsigned int subdomain, unsigned int var) const;

  /**
   * Add a kernel's coupling mask to the blocks a consumer must contract for a (subdomain, variable)
   * pair
   * @param subdomain The contiguous subdomain ID
   * @param var The variable number
   * @param blocks The kernel's QpJacobianBlock mask
   */
  void addBlocks(unsigned int subdomain, unsigned int var, unsigned int blocks);

  /**
   * Zero every allocated tensor, in preparation for a fill
   */
  void zero();

  /**
   * Get the storage occupied by the quadrature-point tensors owned by this process
   * @returns The storage in bytes
   */
  std::size_t localBytes() const;

  /**
   * Copy the outer structures and the coupling masks to device
   */
  void copyToDevice();

  ///@{
  /**
   * Mark the cache as holding (or no longer holding) the linearization of the current solution
   * state. A fill validates; anything that moves the solution state invalidates.
   */
  void invalidate() { _valid = false; }
  void validate() { _valid = true; }
  bool valid() const { return _valid; }
  ///@}

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the tensor of a quadrature point
   * @param subdomain The contiguous subdomain ID
   * @param var The variable number
   * @param qp The subdomain-local flattened quadrature point index
   * @returns The tensor
   */
  KOKKOS_FUNCTION QpJacobianTensor &
  getTensor(unsigned int subdomain, unsigned int var, dof_id_type qp) const
  {
    return _tensors(subdomain, var)[qp];
  }

  /**
   * Get the union of the coupling masks of the kernels active on a (subdomain, variable) pair
   * @param subdomain The contiguous subdomain ID
   * @param var The variable number
   * @returns The QpJacobianBlock mask
   */
  KOKKOS_FUNCTION unsigned int getBlocks(unsigned int subdomain, unsigned int var) const
  {
    return _blocks(subdomain, var);
  }
#endif

private:
  /**
   * Quadrature-point tensors of each (subdomain, variable) pair
   */
  Array2D<Array<QpJacobianTensor>> _tensors;

  /**
   * Union of the coupling masks of the kernels active on each (subdomain, variable) pair
   */
  Array2D<unsigned int> _blocks;

  /**
   * Whether the cache holds the linearization of the current solution state
   */
  bool _valid = false;
};

} // namespace Moose::Kokkos
