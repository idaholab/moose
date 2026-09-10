//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDofSpace.h"

namespace libMesh
{
class System;
}

namespace Moose::Kokkos
{

/**
 * Largest entity block the smoother carries, which bounds the per-thread storage a block solve
 * needs. The interior block of a quadrilateral at order p holds (p - 1)^2 modes and of a hexahedron
 * (p - 1)^3, so this admits order nine in two dimensions and order five in three.
 */
constexpr unsigned int MAX_BLOCK_DOF = 64;

/**
 * The entity-block decomposition of a level's degrees of freedom, together with the diagonal blocks
 * of the level's operator over it.
 *
 * One block is the degrees of freedom a single mesh entity carries for one variable. libMesh
 * already associates every DOF with the entity that owns it -- a vertex, an edge or face node, or
 * the element itself -- so the decomposition is read off the DOF map rather than constructed: for a
 * hierarchic basis at order p on a quadrilateral, a vertex node carries one mode, each edge node
 * carries p - 1, and the element carries the (p - 1)^2 interior modes. Every DOF belongs to exactly
 * one entity, so the blocks are disjoint and the decomposition is a direct sum.
 *
 * Inverting these blocks is what reaches the modes a point smoother cannot. At high order the many
 * basis functions an element carries are strongly coupled to one another, and a smoother built from
 * the operator diagonal alone ignores that coupling entirely, so the error interior to an element
 * and on the interface skeleton between elements is damped by neither the smoother nor the coarse
 * spaces, which do not represent it. Blocking by entity treats the interior modes and each shared
 * edge or face as a unit, which is the standard remedy for high-order p-multigrid; for a Lagrange
 * basis every entity carries one mode per variable, so the decomposition degenerates to the point
 * smoother it generalizes.
 *
 * A block is built and held by the process that owns the entity's degrees of freedom, and gathers
 * only the contributions of the elements that process has, so a block on a partition boundary
 * carries the contributions of its local elements alone. That makes the smoother's strength depend
 * mildly on the partitioning, in the way any Schwarz smoother assembled from process-local data
 * does, and keeps the blocks free of communication.
 */
class EntityBlocks
{
public:
  /**
   * Build the decomposition. The DOFs of the layout's system must already be distributed.
   * @param dof_space The device DOF layout the blocks are indexed by, whose DOF map associates each
   * DOF with the mesh entity carrying it
   * @param mesh The mesh whose entities are walked
   * @param constrained_dof Local-plus-ghost mask of the rows the level holds fixed, which may be
   * unallocated when the level constrains no row
   */
  void init(const DofSpace & dof_space,
            const libMesh::MeshBase & mesh,
            const Array<bool> & constrained_dof);

  /**
   * Get whether the decomposition has been built
   * @returns Whether the decomposition has been built
   */
  bool isBuilt() const { return _built; }

  /**
   * Get the number of blocks this process holds
   * @returns The number of blocks
   */
  dof_id_type numBlocks() const { return _num_blocks; }

  /**
   * Get the number of degrees of freedom of the largest block this process holds
   * @returns The largest block size
   */
  unsigned int maxBlockSize() const { return _max_block_size; }

  /**
   * Zero the block entries, which the operator's block loop then accumulates into
   */
  void zero();

  /**
   * Factor every block in place, so that one factorization serves the many applications a smoother
   * makes of it. A block that is not positive definite is left unfactored and is applied as its
   * diagonal instead, which keeps a smoother built from the blocks defined whatever the blocks
   * hold.
   */
  void factor();

  /**
   * Get the number of blocks that failed to factor and are applied as their diagonal
   * @returns The number of unfactored blocks
   */
  dof_id_type numUnfactoredBlocks() const;

  /**
   * Copy the block entries back to host, so that a host-side check can read them. Meaningful only
   * before the blocks are factored, which overwrites the entries with their factor.
   */
  void copyEntriesToHost();

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Kokkos function tags for the loops over blocks
   */
  ///@{
  struct FactorLoop
  {
  };
  ///@}

  /**
   * Get the number of local DOFs the decomposition covers
   * @returns The number of local DOFs
   */
  KOKKOS_FUNCTION dof_id_type getNumLocalDofs() const { return _num_local_dofs; }

  /**
   * Get whether a DOF belongs to a block this process holds, which a DOF this process does not own
   * and a DOF the level holds fixed do not
   * @param dof The local DOF index
   * @returns Whether the DOF belongs to a block
   */
  KOKKOS_FUNCTION bool isBlocked(const dof_id_type dof) const
  {
    return dof < _num_local_dofs && _block_of_dof[dof] != libMesh::DofObject::invalid_id;
  }

  /**
   * Get the block a DOF belongs to
   * @param dof The local DOF index, which must belong to a block
   * @returns The block index
   */
  KOKKOS_FUNCTION dof_id_type blockOfDof(const dof_id_type dof) const { return _block_of_dof[dof]; }

  /**
   * Get the position a DOF occupies within its own block
   * @param dof The local DOF index, which must belong to a block
   * @returns The position within the block
   */
  KOKKOS_FUNCTION unsigned int posOfDof(const dof_id_type dof) const { return _pos_of_dof[dof]; }

  /**
   * Get the number of degrees of freedom a block holds
   * @param block The block index
   * @returns The block size
   */
  KOKKOS_FUNCTION unsigned int blockSize(const dof_id_type block) const
  {
    return _block_size[block];
  }

  /**
   * Get the local DOF index of one member of a block
   * @param block The block index
   * @param i The position within the block
   * @returns The local DOF index
   */
  KOKKOS_FUNCTION dof_id_type blockDof(const dof_id_type block, const unsigned int i) const
  {
    return _block_dofs[_dof_offset[block] + i];
  }

  /**
   * Get one entry of a block, which is stored row-major
   * @param block The block index
   * @param i The row position within the block
   * @param j The column position within the block
   * @returns The entry
   */
  KOKKOS_FUNCTION Real &
  entry(const dof_id_type block, const unsigned int i, const unsigned int j) const
  {
    return _entries[_entry_offset[block] + j + _block_size[block] * i];
  }

  /**
   * Get whether a block carries a Cholesky factor, as against being applied as its diagonal
   * @param block The block index
   * @returns Whether the block is factored
   */
  KOKKOS_FUNCTION bool isFactored(const dof_id_type block) const { return _factored[block]; }

  /**
   * Get the entries of a block as a contiguous row-major array, which is what a substitution reads
   * @param block The block index
   * @returns The block entries
   */
  KOKKOS_FUNCTION const Real * blockEntries(const dof_id_type block) const
  {
    return &_entries[_entry_offset[block]];
  }

  KOKKOS_FUNCTION void operator()(FactorLoop, const dof_id_type block) const;
#endif

private:
  /**
   * Whether the decomposition has been built
   */
  bool _built = false;

  /**
   * Number of blocks this process holds
   */
  dof_id_type _num_blocks = 0;

  /**
   * Number of local DOFs the decomposition covers
   */
  dof_id_type _num_local_dofs = 0;

  /**
   * Size of the largest block this process holds
   */
  unsigned int _max_block_size = 0;

  /**
   * Number of DOFs each block holds
   */
  Array<unsigned int> _block_size;

  /**
   * Offset of each block into the entry array, with a trailing total
   */
  Array<dof_id_type> _entry_offset;

  /**
   * Offset of each block into the member array, with a trailing total
   */
  Array<dof_id_type> _dof_offset;

  /**
   * Local DOF index of each block member, blocks laid end to end
   */
  Array<dof_id_type> _block_dofs;

  /**
   * Entries of every block, row-major, blocks laid end to end
   */
  Array<Real> _entries;

  /**
   * Whether each block carries a Cholesky factor
   */
  Array<bool> _factored;

  /**
   * Block each local DOF belongs to, invalid where it belongs to none
   */
  Array<dof_id_type> _block_of_dof;

  /**
   * Position each local DOF occupies within its own block
   */
  Array<unsigned int> _pos_of_dof;
};

} // namespace Moose::Kokkos
