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
 * A block is held by the process that owns the entity's degrees of freedom, and its entries gather
 * a contribution from every element carrying that entity. The block loop reaches only the elements
 * a process has, so a process also fills a block for each entity its elements carry that another
 * process owns, and reduce() sums those into the owner once the loop has run. The decomposition is
 * therefore by entity rather than by process, and the operator the smoother applies is the same one
 * whatever the partitioning: an iteration count that grows with the process count means a block is
 * missing a contribution rather than that the smoother has weakened.
 */
class EntityBlocks
{
public:
  /**
   * Build the decomposition. The DOFs of the layout's system must already be distributed.
   * @param dof_space The device DOF layout the blocks are indexed by, whose DOF map associates each
   * DOF with the mesh entity carrying it
   * @param mesh The mesh whose entities are walked
   * @param solution_vector The ghosted system solution, which maps a global DOF index to the local
   * index the block loop and the level's vectors are indexed by
   * @param constrained_dof Local-plus-ghost mask of the rows the level holds fixed, which may be
   * unallocated when the level constrains no row
   */
  void init(const DofSpace & dof_space,
            const libMesh::MeshBase & mesh,
            libMesh::NumericVector<Number> & solution_vector,
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
   * Sum each block this process fills for an entity it does not own into the block the owning
   * process holds for that entity. An entity's block gathers a contribution from every element
   * carrying the entity, and the block loop reaches only this process's own elements, so a block on
   * a partition boundary is complete only once the processes sharing the entity have added theirs.
   * Called once the block loop has accumulated, and before the blocks are factored or read.
   */
  void reduce();

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
   * Get whether a DOF belongs to a block this process fills. A DOF this process owns belongs to one
   * of the blocks it holds, and a ghosted DOF to one of the blocks it fills for the owning process
   * and reduces into it; a DOF the level holds fixed belongs to none.
   * @param dof The local DOF index
   * @returns Whether the DOF belongs to a block
   */
  KOKKOS_FUNCTION bool isBlocked(const dof_id_type dof) const
  {
    return dof < _num_dofs && _block_of_dof[dof] != libMesh::DofObject::invalid_id;
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
   * Number of blocks this process holds, which are the blocks of the entities it owns and the ones
   * the smoother applies and factors. They come first in every per-block array.
   */
  dof_id_type _num_blocks = 0;

  /**
   * Number of blocks the block loop fills, the blocks this process holds followed by one for each
   * entity its own elements carry but another process owns. A trailing block is summed into its
   * owner by reduce() and is never applied here.
   */
  dof_id_type _num_all_blocks = 0;

  /**
   * Number of local DOFs the decomposition covers
   */
  dof_id_type _num_local_dofs = 0;

  /**
   * Number of local plus ghost DOFs, which is the index space the block loop reaches and so the
   * extent of the per-DOF arrays below
   */
  dof_id_type _num_dofs = 0;

  /**
   * Communicator the reduction of the trailing blocks is carried out over
   */
  const libMesh::Parallel::Communicator * _comm = nullptr;

  /**
   * Global index of the first DOF of each block this process holds, which names the block to the
   * processes that reduce into it. Both sides build a block's members from the same entity in
   * component order, so the first member identifies the block on either side.
   */
  Array<dof_id_type> _owned_block_key;

  /**
   * Global index of the first DOF of each block this process fills for another process, ordered as
   * those blocks are
   */
  Array<dof_id_type> _ghost_block_key;

  /**
   * Process owning each block this process fills for another process
   */
  Array<libMesh::processor_id_type> _ghost_block_owner;

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
