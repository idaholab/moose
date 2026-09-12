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
  _local_to_global_dof_index.create(_num_local_dofs + _num_ghost_dofs);

  _max_dofs_per_elem.create(_num_vars);
  _max_dofs_per_elem = 0;

  auto solution = dynamic_cast<libMesh::PetscVector<Number> *>(system.current_local_solution.get());

#ifdef MOOSE_ENABLE_KOKKOS_GPU
  // Kokkos array thinks OpenMP clause is device code when using OpenMP backend
#pragma omp parallel for
#endif
  for (unsigned int var = 0; var < _num_vars; ++var)
  {
    std::vector<dof_id_type> dof_indices;

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
        _local_elem_dof_index[var](dof, id) = solution->map_global_to_local_index(dof_indices[dof]);
        _local_to_global_dof_index[_local_elem_dof_index[var](dof, id)] = dof_indices[dof];
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

  _local_elem_dof_index.copyToDeviceNested();
  _local_to_global_dof_index.copyToDevice();

  _max_dofs_per_elem.copyToDevice();

  // Setup DOF communication maps

  auto num_procs = _comm.size();

  std::vector<std::vector<dof_id_type>> send_list(num_procs);
  std::vector<std::vector<dof_id_type>> recv_list(num_procs);

  for (auto dof : _dof_map.get_send_list())
    recv_list[_dof_map.dof_owner(dof)].push_back(dof);

  for (processor_id_type proc = 0; proc < num_procs; proc++)
    _comm.scatter(recv_list, send_list[proc], proc);

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
  std::vector<PetscInt> row_idx;
  std::vector<PetscInt> row_ptr = {0};

  for (dof_id_type r = _dof_map.first_dof(); r < _dof_map.end_dof(); ++r)
  {
    auto & cols = pattern->get_sparsity_pattern().at(r - _dof_map.first_dof());

    for (auto col : cols)
    {
      col_idx.push_back(col);
      row_idx.push_back(r);
    }

    row_ptr.push_back(col_idx.size());
  }

  auto num_procs = _comm.size();

  std::vector<std::vector<PetscInt>> send_cols(num_procs), send_num_cols(num_procs);
  std::vector<std::vector<PetscInt>> recv_cols(num_procs), recv_num_cols(num_procs);

  for (processor_id_type proc = 0; proc < num_procs; proc++)
    for (auto r : _local_comm_list[proc])
    {
      for (PetscInt c = row_ptr[r]; c < row_ptr[r + 1]; ++c)
        send_cols[proc].push_back(col_idx[c]);

      send_num_cols[proc].push_back(row_ptr[r + 1] - row_ptr[r]);
    }

  for (processor_id_type proc = 0; proc < num_procs; proc++)
  {
    _comm.scatter(send_cols, recv_cols[proc], proc);
    _comm.scatter(send_num_cols, recv_num_cols[proc], proc);
  }

  std::vector<PetscInt> col(num_procs), row(num_procs);

  for (auto r : _dof_map.get_send_list())
  {
    auto proc = _dof_map.dof_owner(r);
    auto n_cols = recv_num_cols[proc][row[proc]++];

    for (PetscInt c = 0; c < n_cols; ++c)
    {
      col_idx.push_back(recv_cols[proc][col[proc]++]);
      row_idx.push_back(r);
    }

    row_ptr.push_back(col_idx.size());
  }

  _sparsity.col_idx = col_idx;
  _sparsity.row_idx = row_idx;
  _sparsity.row_ptr = row_ptr;
}

void
DofSpace::syncGhostMask(Array<bool> & mask) const
{
  const auto num_procs = _comm.size();

  std::vector<std::vector<char>> send(num_procs), recv(num_procs);

  for (processor_id_type proc = 0; proc < num_procs; proc++)
    for (auto dof : _local_comm_list[proc])
      send[proc].push_back(mask[dof]);

  for (processor_id_type proc = 0; proc < num_procs; proc++)
    _comm.scatter(send, recv[proc], proc);

  for (processor_id_type proc = 0; proc < num_procs; proc++)
    for (dof_id_type i = 0; i < _ghost_comm_list[proc].size(); ++i)
      mask[_ghost_comm_list[proc][i]] = recv[proc][i];
}

} // namespace Moose::Kokkos
