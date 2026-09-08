//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosDofSpace.h"

#include "MooseMesh.h"

#include "libmesh/system.h"
#include "libmesh/dof_map.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/int_range.h"

#include "timpi/parallel_sync.h"

#include <algorithm>
#include <map>

namespace Moose::Kokkos
{

DofSpace::DofSpace(const MooseMesh & mesh, libMesh::System & system)
  : MeshHolder(*mesh.getKokkosMesh()),
    _mesh(mesh),
    _dof_map(system.get_dof_map()),
    _comm(system.get_dof_map().comm()),
    _num_vars(system.n_vars()),
    _num_local_dofs(system.n_local_dofs()),
    _num_ghost_dofs(system.get_dof_map().get_send_list().size())
{
  setupDofs(system);
}

void
DofSpace::setupDofs(libMesh::System & system)
{
  const auto num_total_elems = kokkosMesh().getNumLocalAndPossiblyOneNeighborLayerGhostElements();

  _local_elem_dof_index.create(_num_vars);
  _scalar_dof_index.create(_num_vars);
  _local_to_global_dof_index.create(_num_local_dofs + _num_ghost_dofs);

  _max_dofs_per_elem.create(_num_vars);
  _max_dofs_per_elem = 0;

  _var_is_scalar.create(_num_vars);
  _var_is_scalar = false;

  auto solution = dynamic_cast<libMesh::PetscVector<Number> *>(system.current_local_solution.get());

#ifdef MOOSE_ENABLE_KOKKOS_GPU
  // Kokkos array thinks OpenMP clause is device code when using OpenMP backend
#pragma omp parallel for
#endif
  for (unsigned int var = 0; var < _num_vars; ++var)
  {
    _var_is_scalar[var] = system.variable_type(var).family == libMesh::FEFamily::SCALAR;

    std::vector<dof_id_type> dof_indices;

    if (!_var_is_scalar[var])
    {
      for (const auto elem : _mesh.getMesh().active_local_element_ptr_range())
      {
        _dof_map.dof_indices(elem, dof_indices, var);

        _max_dofs_per_elem[var] =
            std::max(_max_dofs_per_elem[var], static_cast<unsigned int>(dof_indices.size()));
      }

      _local_elem_dof_index[var].create(_max_dofs_per_elem[var], num_total_elems);
      _local_elem_dof_index[var] = libMesh::DofObject::invalid_id;

      for (const auto elem : _mesh.getMesh().active_local_element_ptr_range())
      {
        const auto id = kokkosMesh().getContiguousElementID(elem);

        _dof_map.dof_indices(elem, dof_indices, var);

        for (unsigned int dof = 0; dof < dof_indices.size(); ++dof)
        {
          const auto local_idx = solution->map_global_to_local_index(dof_indices[dof]);
          _local_elem_dof_index[var](dof, id) = local_idx;
          _local_to_global_dof_index[local_idx] = dof_indices[dof];
        }
      }

      for (auto & [ghost_elem, ghost_id] : kokkosMesh().getGhostElemIdMapping())
      {
        if (!_dof_map.is_evaluable(*ghost_elem, var))
          continue;

        _dof_map.dof_indices(ghost_elem, dof_indices, var);

        for (const auto dof : index_range(dof_indices))
        {
          const auto local_idx = solution->map_global_to_local_index(dof_indices[dof]);
          _local_elem_dof_index[var](dof, ghost_id) = local_idx;
          _local_to_global_dof_index[local_idx] = dof_indices[dof];
        }
      }
    }
    else if (num_total_elems)
    {
      _dof_map.SCALAR_dof_indices(dof_indices, var);
      _scalar_dof_index[var].create(dof_indices.size());

      for (const auto dof : index_range(dof_indices))
      {
        const auto local_idx = solution->map_global_to_local_index(dof_indices[dof]);
        _scalar_dof_index[var][dof] = local_idx;
        _local_to_global_dof_index[local_idx] = dof_indices[dof];
      }
    }
  }

  _local_elem_dof_index.copyToDeviceNested();
  _scalar_dof_index.copyToDeviceNested();
  _local_to_global_dof_index.copyToDevice();

  _max_dofs_per_elem.copyToDevice();

  _var_is_scalar.copyToDevice();

  // Setup DOF communication maps

  auto num_procs = _comm.size();

  std::vector<std::vector<dof_id_type>> send_list(num_procs);
  std::vector<std::vector<dof_id_type>> recv_list(num_procs);

  // Tell the owner of each ghost DOF that we need it. This is a sparse exchange with only the
  // neighboring processes, so its cost does not grow with the total number of processes.
  {
    std::map<processor_id_type, std::vector<dof_id_type>> push_data;

    for (auto dof : _dof_map.get_send_list())
      push_data[_dof_map.dof_owner(dof)].push_back(dof);

    for (const auto & [proc, dofs] : push_data)
      recv_list[proc] = dofs;

    auto receive = [&send_list](const processor_id_type pid, std::vector<dof_id_type> && dofs)
    { send_list[pid] = std::move(dofs); };

    TIMPI::push_parallel_vector_data(_comm, std::move(push_data), receive);
  }

  _local_comm_list.create(num_procs);
  _ghost_comm_list.create(num_procs);

  for (processor_id_type proc = 0; proc < num_procs; proc++)
  {
    _local_comm_list[proc].create(send_list[proc].size());
    _ghost_comm_list[proc].create(recv_list[proc].size());

    for (dof_id_type i = 0; i < send_list[proc].size(); ++i)
      _local_comm_list[proc][i] = solution->map_global_to_local_index(send_list[proc][i]);

    for (dof_id_type i = 0; i < recv_list[proc].size(); ++i)
      _ghost_comm_list[proc][i] = solution->map_global_to_local_index(recv_list[proc][i]);
  }

  _local_comm_list.copyToDeviceNested();
  _ghost_comm_list.copyToDeviceNested();
}

void
DofSpace::setupSparsity()
{
  auto * const pattern = _dof_map.get_sparsity_pattern();
  bool have_pattern = pattern;

  // Make sure to check whether the sparsity pattern graph returned by
  // pattern->get_sparsity_pattern() is non-empty
  getComm().verify(have_pattern);
  if (!have_pattern)
    return;
  have_pattern = !pattern->get_sparsity_pattern().empty();
  getComm().max(have_pattern);
  if (!have_pattern)
    return;

  std::vector<PetscInt> col_idx;
  std::vector<PetscInt> row_ptr = {0};

  // The rows are laid out as the locally owned rows in ascending order followed by the ghost rows
  // in send list order.  buildCooIndices() recovers each row's global index from that ordering, so
  // the two must be kept consistent.
  for (dof_id_type r = _dof_map.first_dof(); r < _dof_map.end_dof(); ++r)
  {
    auto & cols = pattern->get_sparsity_pattern().at(r - _dof_map.first_dof());

    for (auto col : cols)
      col_idx.push_back(col);

    row_ptr.push_back(col_idx.size());
  }

  auto num_procs = _comm.size();

  // Send the sparsity of the rows ghosted by each neighboring process to it, packed as
  // [n_cols, cols..., n_cols, cols..., ...] in the order of its ghost DOFs. This is a sparse
  // exchange with only the neighboring processes.
  std::map<processor_id_type, std::vector<PetscInt>> send_rows;

  for (const auto proc : make_range(num_procs))
    for (auto r : _local_comm_list[proc])
    {
      auto & send = send_rows[proc];

      send.push_back(row_ptr[r + 1] - row_ptr[r]);

      for (PetscInt c = row_ptr[r]; c < row_ptr[r + 1]; ++c)
        send.push_back(col_idx[c]);
    }

  std::vector<std::vector<PetscInt>> recv_rows(num_procs);

  auto receive = [&recv_rows](const processor_id_type pid, std::vector<PetscInt> && rows)
  { recv_rows[pid] = std::move(rows); };

  TIMPI::push_parallel_vector_data(_comm, std::move(send_rows), receive);

  std::vector<std::size_t> pos(num_procs);

  for (auto r : _dof_map.get_send_list())
  {
    auto proc = _dof_map.dof_owner(r);
    auto & rows = recv_rows[proc];
    auto n_cols = rows[pos[proc]++];

    for (PetscInt c = 0; c < n_cols; ++c)
      col_idx.push_back(rows[pos[proc]++]);

    row_ptr.push_back(col_idx.size());
  }

  _sparsity.col_idx = col_idx;
  _sparsity.row_ptr = row_ptr;
}

void
DofSpace::buildCooIndices(std::vector<PetscInt> & coo_i, std::vector<PetscInt> & coo_j) const
{
  const auto & col_idx = _sparsity.col_idx;
  const auto & row_ptr = _sparsity.row_ptr;

  coo_i.resize(col_idx.size());
  coo_j.assign(&col_idx.begin(), &col_idx.end());

  // Every nonzero of a row carries that row's global index, which setupSparsity() leaves implicit
  // in the row ordering.  The two loops below walk the rows in the order it wrote them.
  dof_id_type row = 0;

  auto fill_row = [&](const PetscInt global_row)
  {
    std::fill(coo_i.begin() + row_ptr[row], coo_i.begin() + row_ptr[row + 1], global_row);
    ++row;
  };

  for (const auto global_row : make_range(_dof_map.first_dof(), _dof_map.end_dof()))
    fill_row(global_row);

  for (const auto global_row : _dof_map.get_send_list())
    fill_row(global_row);

  mooseAssert(row == row_ptr.size() - 1,
              "Kokkos DOF space error: the DOF map no longer describes the rows that setupSparsity() "
              "recorded.");
}

void
DofSpace::syncGhostMask(Array<bool> & mask) const
{
  const auto num_procs = _comm.size();

  // A sparse exchange with only the neighboring processes
  std::map<processor_id_type, std::vector<char>> send;

  for (const auto proc : make_range(num_procs))
    for (auto dof : _local_comm_list[proc])
      send[proc].push_back(mask[dof]);

  auto receive = [this, &mask](const processor_id_type pid, const std::vector<char> & recv)
  {
    const auto & ghost_dofs = _ghost_comm_list[pid];

    for (const auto i : index_range(ghost_dofs))
      mask[ghost_dofs[i]] = recv[i];
  };

  TIMPI::push_parallel_vector_data(_comm, std::move(send), receive);
}

} // namespace Moose::Kokkos
