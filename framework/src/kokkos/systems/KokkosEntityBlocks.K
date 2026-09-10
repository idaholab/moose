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
#include "libmesh/system.h"

#include <algorithm>
#include <set>
#include <vector>

namespace Moose::Kokkos
{

void
EntityBlocks::init(const DofSpace & dof_space,
                   const libMesh::MeshBase & mesh,
                   const Array<bool> & constrained_dof)
{
  const auto & dof_map = dof_space.getDofMap();
  const auto sys_num = dof_map.sys_number();
  const auto first_dof = dof_map.first_dof();
  const auto end_dof = dof_map.end_dof();
  const auto num_vars = dof_space.getNumVars();

  _num_local_dofs = dof_space.getNumLocalDofs();

  // The members of each block, as local DOF indices, gathered before the flat device arrays are
  // sized, since the number of blocks a process holds is not known until the entities are walked
  std::vector<std::vector<dof_id_type>> blocks;

  // One entity contributes one block per variable: the DOFs it carries for that variable, less the
  // DOFs this process does not own and the DOFs the level holds fixed. A fixed row carries the
  // identity in the level's operator, so leaving it out of every block is what lets the smoother
  // reproduce that identity rather than invert a block that is singular for having a fixed row in
  // it.
  const auto add_entity = [&](const libMesh::DofObject & entity, const unsigned int var)
  {
    const auto num_comps = entity.n_comp(sys_num, var);

    std::vector<dof_id_type> dofs;

    for (const auto comp : make_range(num_comps))
    {
      const auto global = entity.dof_number(sys_num, var, comp);

      if (global < first_dof || global >= end_dof)
        continue;

      const auto local = global - first_dof;

      if (constrained_dof.isAlloc() && constrained_dof[local])
        continue;

      dofs.push_back(local);
    }

    if (!dofs.empty())
      blocks.push_back(std::move(dofs));
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

  for (const auto & block : blocks)
    _max_block_size = std::max(_max_block_size, static_cast<unsigned int>(block.size()));

  if (_max_block_size > MAX_BLOCK_DOF)
    mooseError("An entity-block smoother block of ",
               _max_block_size,
               " degrees of freedom exceeds the largest block the smoother carries, ",
               MAX_BLOCK_DOF,
               ". The interior block of an element is the largest, so this configuration needs "
               "either a lower polynomial order or a larger MAX_BLOCK_DOF.");

  _block_size.create(_num_blocks);
  _entry_offset.create(_num_blocks + 1);
  _dof_offset.create(_num_blocks + 1);
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

  _block_dofs.create(_dof_offset[_num_blocks]);
  _entries.create(_entry_offset[_num_blocks]);

  _block_of_dof.create(_num_local_dofs);
  _pos_of_dof.create(_num_local_dofs);

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
  _entries = 0;
  _entries.copyToDevice();
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
