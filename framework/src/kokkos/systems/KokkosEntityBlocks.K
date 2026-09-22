//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosEntityBlocks.h"
#include "KokkosUtils.h"

#include "MooseError.h"

#include "libmesh/dof_map.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_base.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/parallel_sync.h"
#include "libmesh/system.h"

#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace Moose::Kokkos
{

void
EntityBlocks::init(const DofSpace & dof_space,
                   const libMesh::MeshBase & mesh,
                   libMesh::NumericVector<Number> & solution_vector,
                   const Array<bool> & constrained_dof)
{
  const auto & dof_map = dof_space.getDofMap();
  const auto sys_num = dof_map.sys_number();
  const auto num_vars = dof_space.getNumVars();

  _num_local_dofs = dof_space.getNumLocalDofs();
  _num_dofs = _num_local_dofs + dof_space.getNumGhostDofs();
  _comm = &dof_space.getComm();

  // The block loop reaches every DOF of a local element, the ghosted ones included, so a block
  // built for a ghosted entity needs the index each of its DOFs occupies in that space. That index
  // is the one the ghosted solution assigns, which is what the DOF layout numbers a local element's
  // DOFs by and so what the level's vectors are addressed by.
  auto * const solution = libMesh::cast_ptr<libMesh::PetscVector<Number> *>(&solution_vector);

  // The members of each block, as local DOF indices, gathered before the flat device arrays are
  // sized, since the number of blocks a process holds is not known until the entities are walked
  std::vector<std::vector<dof_id_type>> blocks;
  // Global index of the first member of each block, which names the block across processes
  std::vector<dof_id_type> keys;
  ///@{
  // The same, for the entities a local element carries that another process owns. They are kept
  // apart so that they follow the blocks this process holds, which are the ones it applies.
  std::vector<std::vector<dof_id_type>> ghost_blocks;
  std::vector<dof_id_type> ghost_keys;
  std::vector<libMesh::processor_id_type> ghost_owners;
  ///@}

  // One entity contributes one block per variable: the DOFs it carries for that variable, less the
  // DOFs the level holds fixed. A fixed row carries the identity in the level's operator, so
  // leaving it out of every block is what lets the smoother reproduce that identity rather than
  // invert a block that is singular for having a fixed row in it.
  //
  // A block is either wholly owned here or wholly owned by one other process. The latter is built
  // too: this process's elements carry part of its entries, and reduce() sends them to the owner.
  // Both sides walk the entity's components in the same order and drop the same fixed rows, so the
  // two agree on the block's membership and on the global index of its first member. Ownership is
  // read from the DOF layout, which is what the block loop and the vectors are indexed by, so that
  // it cannot disagree with the index a member occupies.
  const auto add_entity = [&](const libMesh::DofObject & entity, const unsigned int var)
  {
    const auto num_comps = entity.n_comp(sys_num, var);

    std::vector<dof_id_type> dofs;
    auto key = libMesh::DofObject::invalid_id;

    for (const auto comp : make_range(num_comps))
    {
      const auto global = entity.dof_number(sys_num, var, comp);
      const auto local = solution->map_global_to_local_index(global);

      if (constrained_dof.isAlloc() && constrained_dof[local])
        continue;

      if (dofs.empty())
        key = global;

      dofs.push_back(local);
    }

    if (dofs.empty())
      return;

    const auto owned = dofs.front() < _num_local_dofs;

    for (const auto local : dofs)
      if ((local < _num_local_dofs) != owned)
        mooseError("The entity carrying global degree of freedom ",
                   key,
                   " holds degrees of freedom owned by more than one process, so its block is "
                   "neither one this process holds nor one it fills for a single other process.");

    if (owned)
    {
      blocks.push_back(std::move(dofs));
      keys.push_back(key);
    }
    else
    {
      ghost_blocks.push_back(std::move(dofs));
      ghost_keys.push_back(key);
      ghost_owners.push_back(dof_map.dof_owner(key));
    }
  };

  // libMesh associates a DOF with the entity that carries it, so walking the entities of every
  // local element -- the nodes, which carry the vertex modes and the edge and face modes, and the
  // element itself, which carries the interior modes -- enumerates the decomposition directly. A
  // node is shared between elements, so it is taken the first time it is reached.
  std::set<dof_id_type> visited;

  for (const auto * const elem : mesh.active_local_element_ptr_range())
  {
    for (const auto var : make_range(num_vars))
      add_entity(*elem, var);

    for (const auto i : elem->node_index_range())
    {
      const auto & node = elem->node_ref(i);

      if (visited.insert(node.id()).second)
        for (const auto var : make_range(num_vars))
          add_entity(node, var);
    }
  }

  _num_blocks = blocks.size();

  // The blocks built for another process's entities follow the ones this process holds, so that a
  // block index below _num_blocks is one the smoother applies and factors and the rest are the ones
  // reduce() sends away
  for (auto & block : ghost_blocks)
    blocks.push_back(std::move(block));

  _num_all_blocks = blocks.size();

  _owned_block_key = keys;
  _ghost_block_key = ghost_keys;
  _ghost_block_owner = ghost_owners;

  for (const auto & block : blocks)
    _max_block_size = std::max(_max_block_size, static_cast<unsigned int>(block.size()));

  if (_max_block_size > MAX_BLOCK_DOF)
    mooseError("An entity-block smoother block of ",
               _max_block_size,
               " degrees of freedom exceeds the largest block the smoother carries, ",
               MAX_BLOCK_DOF,
               ". The interior block of an element is the largest, so this configuration needs "
               "either a lower polynomial order or a larger MAX_BLOCK_DOF.");

  _block_size.create(_num_all_blocks);
  _entry_offset.create(_num_all_blocks + 1);
  _dof_offset.create(_num_all_blocks + 1);
  _factored.create(_num_blocks);

  _entry_offset[0] = 0;
  _dof_offset[0] = 0;

  for (const auto block : index_range(blocks))
  {
    const auto size = blocks[block].size();

    _block_size[block] = size;
    _entry_offset[block + 1] = _entry_offset[block] + size * size;
    _dof_offset[block + 1] = _dof_offset[block] + size;
  }

  _block_dofs.create(_dof_offset[_num_all_blocks]);
  _entries.create(_entry_offset[_num_all_blocks]);

  _block_of_dof.create(_num_dofs);
  _pos_of_dof.create(_num_dofs);

  _block_of_dof = libMesh::DofObject::invalid_id;
  _pos_of_dof = 0;

  for (const auto block : index_range(blocks))
    for (const auto i : index_range(blocks[block]))
    {
      const auto dof = blocks[block][i];

      _block_dofs[_dof_offset[block] + i] = dof;
      _block_of_dof[dof] = block;
      _pos_of_dof[dof] = i;
    }

  _block_size.copyToDevice();
  _entry_offset.copyToDevice();
  _dof_offset.copyToDevice();
  _block_dofs.copyToDevice();
  _block_of_dof.copyToDevice();
  _pos_of_dof.copyToDevice();

  _built = true;
}

void
EntityBlocks::zero()
{
  // create() allocates both sides and assigning a scalar fills whichever sides an array holds, so
  // this zeroes the device entries in place. Uploading the host zeros on top of them would move the
  // whole entry array across the bus, several gigabytes for an order-eight quadrilateral, to no
  // effect: the block loop accumulates on the device, and reduce() and copyEntriesToHost() are what
  // bring the entries back to the host.
  _entries = 0;
}

void
EntityBlocks::reduce()
{
  if (_comm->size() == 1)
    return;

  // The block loop accumulates on the device, and the sum below is carried out on the host
  _entries.copyToHost();

  // One record per block sent: its key, its size, and its entries row-major. A key is a global DOF
  // index and a size is a block extent, each of which a Real holds exactly, so the record needs one
  // buffer rather than a packing of two types.
  std::map<libMesh::processor_id_type, std::vector<Real>> sends;

  for (const auto block : make_range(_num_blocks, _num_all_blocks))
  {
    const auto size = _block_size[block];

    auto & buffer = sends[_ghost_block_owner[block - _num_blocks]];

    buffer.push_back(_ghost_block_key[block - _num_blocks]);
    buffer.push_back(size);

    for (const auto i : make_range(size))
      for (const auto j : make_range(size))
        buffer.push_back(entry(block, i, j));
  }

  std::unordered_map<dof_id_type, dof_id_type> block_of_key;

  for (const auto block : make_range(_num_blocks))
    block_of_key[_owned_block_key[block]] = block;

  const auto accumulate =
      [&](const libMesh::processor_id_type from, const std::vector<Real> & received)
  {
    std::size_t k = 0;

    while (k < received.size())
    {
      const auto key = static_cast<dof_id_type>(received[k++]);
      const auto size = static_cast<unsigned int>(received[k++]);

      const auto it = block_of_key.find(key);

      // Both sides build a block from the same entity, so a key this process holds no block for
      // means the two disagreed on the decomposition rather than that the entry is unwanted
      if (it == block_of_key.end())
        mooseError("Process ",
                   from,
                   " reduced an entity block keyed on global degree of freedom ",
                   key,
                   " into this process, which owns that degree of freedom but holds no block for "
                   "it.");

      const auto block = it->second;

      if (_block_size[block] != size)
        mooseError("Process ",
                   from,
                   " reduced an entity block of ",
                   size,
                   " degrees of freedom into a block of ",
                   _block_size[block],
                   " on this process. The two disagreed on which of the entity's rows the level "
                   "holds fixed.");

      for (const auto i : make_range(size))
        for (const auto j : make_range(size))
          entry(block, i, j) += received[k++];
    }
  };

  libMesh::Parallel::push_parallel_vector_data(*_comm, sends, accumulate);

  _entries.copyToDevice();
}

void
EntityBlocks::copyEntriesToHost()
{
  _entries.copyToHost();
}

void
EntityBlocks::factor()
{
  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<dof_id_type>, FactorLoop> policy(
      0, _num_blocks);
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();

  _factored.copyToHost();
}

dof_id_type
EntityBlocks::numUnfactoredBlocks() const
{
  dof_id_type unfactored = 0;

  for (const auto block : make_range(_num_blocks))
    if (!_factored[block])
      ++unfactored;

  return unfactored;
}

KOKKOS_FUNCTION void
EntityBlocks::operator()(FactorLoop, const dof_id_type block) const
{
  // A one-by-one block is its own diagonal, so it is left for the diagonal path rather than
  // carrying a factor of a single entry
  if (_block_size[block] == 1)
  {
    _factored[block] = false;
    return;
  }

  _factored[block] = Utils::choleskyFactor(&_entries[_entry_offset[block]], _block_size[block]);
}

} // namespace Moose::Kokkos
