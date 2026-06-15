//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSystem.h"

#include "MooseMesh.h"
#include "FEProblemBase.h"
#include "AuxiliarySystem.h"

#include "libmesh/system.h"

namespace Moose::Kokkos
{

System::System(const System & src)
  : PerfGraphInterface(src._system.feProblem().getMooseApp().perfGraph(), "KokkosSystem"),
    MeshHolder(src),
    _system(src._system),
    _mesh(src._mesh),
    _dof_map(src._dof_map),
    _comm(src._comm),
    _num_vars(src._num_vars),
    _num_local_dofs(src._num_local_dofs),
    _num_ghost_dofs(src._num_ghost_dofs),
    _vectors(src._vectors),
    _matrices(src._matrices),
    _local_elem_dof_index(src._local_elem_dof_index),
    _local_to_global_dof_index(src._local_to_global_dof_index),
    _max_dofs_per_elem(src._max_dofs_per_elem),
    _var_subdomain_active(src._var_subdomain_active),
    _active_variables(src._active_variables),
    _active_solution_tags(src._active_solution_tags),
    _active_residual_tags(src._active_residual_tags),
    _active_matrix_tags(src._active_matrix_tags),
    _residual_tag_active(src._residual_tag_active),
    _matrix_tag_active(src._matrix_tag_active),
    _local_comm_list(src._local_comm_list),
    _ghost_comm_list(src._ghost_comm_list),
    _sparsity(src._sparsity)
{
}

System::System(::SystemBase & system)
  : PerfGraphInterface(system.feProblem().getMooseApp().perfGraph(), "KokkosSystem"),
    MeshHolder(*system.mesh().getKokkosMesh()),
    _system(system),
    _mesh(system.mesh()),
    _dof_map(system.dofMap()),
    _comm(system.dofMap().comm()),
    _num_vars(system.system().n_vars()),
    _num_local_dofs(system.system().n_local_dofs()),
    _num_ghost_dofs(system.dofMap().get_send_list().size())
{
  setupVariables();
  setupDofs();

  if (!dynamic_cast<AuxiliarySystem *>(&_system))
    setupSparsity();
}

void
System::setupVariables()
{
  auto & sys = _system.system();

  _var_subdomain_active.create(_num_vars, kokkosMesh().getNumSubdomains());

  for (unsigned int var = 0; var < _num_vars; ++var)
    for (auto subdomain : _mesh.meshSubdomains())
      _var_subdomain_active(var, kokkosMesh().getContiguousSubdomainID(subdomain)) =
          sys.variable(var).active_on_subdomain(subdomain);

  _var_subdomain_active.copyToDevice();
}

void
System::setupDofs()
{
  auto & sys = _system.system();

  const auto num_total_elems = kokkosMesh().getNumTotalElements();

  _residual_tag_active.create(MAX_TAG);
  _residual_tag_active = false;

  _matrix_tag_active.create(MAX_TAG);
  _matrix_tag_active = false;

  _vectors.create(MAX_TAG);
  _matrices.create(MAX_TAG);

  _local_elem_dof_index.create(_num_vars);
  _local_to_global_dof_index.create(_num_local_dofs + _num_ghost_dofs);

  _max_dofs_per_elem.create(_num_vars);
  _max_dofs_per_elem = 0;

  auto solution = dynamic_cast<PetscVector<Number> *>(sys.current_local_solution.get());

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
System::setupSparsity()
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
System::sync(const MemcpyType dir)
{
  if (dir == MemcpyType::HOST_TO_DEVICE)
  {
    for (auto tag : _active_solution_tags)
    {
      auto & vector = _system.getVector(tag);

      _vectors[tag].create(vector, *this, false);
      _vectors[tag].copyToDevice();
    }

    for (auto tag : _active_residual_tags)
    {
      auto & vector = _system.getVector(tag);

      _vectors[tag].create(vector, *this, true);
      _vectors[tag].copyToDevice();
    }

    for (auto tag : _active_matrix_tags)
    {
      auto & matrix = _system.getMatrix(tag);

      {
        TIME_SECTION("MatSetPreallocation", 1);
        _matrices[tag].create(matrix, *this);
      }

      _matrices[tag] = 0;
    }

    _vectors.copyToDevice();
    _matrices.copyToDevice();
  }
  else if (dir == MemcpyType::DEVICE_TO_HOST)
  {
    for (auto tag : _active_solution_tags)
      _vectors[tag].restore();

    for (auto tag : _active_residual_tags)
      _vectors[tag].close();

    for (auto tag : _active_matrix_tags)
    {
      TIME_SECTION("MatSetValues", 1);
      _matrices[tag].close();
    }
  }
}

void
System::sync(const std::set<TagID> & tags, const MemcpyType dir)
{
  if (dir == MemcpyType::HOST_TO_DEVICE)
  {
    for (auto tag : tags)
    {
      if (!_system.hasVector(tag))
        continue;

      auto & vector = _system.getVector(tag);

      _vectors[tag].create(vector, *this, false);
      _vectors[tag].copyToDevice();
    }

    _vectors.copyToDevice();
  }
  else if (dir == MemcpyType::DEVICE_TO_HOST)
  {
    for (auto tag : tags)
    {
      if (!_system.hasVector(tag))
        continue;

      _vectors[tag].copyToHost();
      _vectors[tag].restore();
    }
  }
}

void
System::sync(const std::vector<TagID> & tags, const MemcpyType dir)
{
  sync(std::set<TagID>(tags.begin(), tags.end()), dir);
}

void
System::sync(const TagID tag, const MemcpyType dir)
{
  sync(std::set<TagID>{tag}, dir);
}

void
System::setActiveVariables(const std::set<MooseVariableFieldBase *> & vars)
{
  std::set<unsigned int> active_variables;

  for (auto var : vars)
    if (var->sys().number() == _system.number())
      for (unsigned int i = 0; i < var->count(); ++i)
        active_variables.insert(var->number() + i);

  _active_variables = active_variables;
}

void
System::setActiveSolutionTags(const std::set<TagID> & tags)
{
  std::set<TagID> active_solution_tags;

  for (auto tag : tags)
    if (_system.hasVector(tag))
      active_solution_tags.insert(tag);

  _active_solution_tags = active_solution_tags;
}

void
System::setActiveResidualTags(const std::set<TagID> & tags)
{
  std::set<TagID> active_residual_tags;

  for (auto tag : tags)
    if (_system.hasVector(tag))
    {
      active_residual_tags.insert(tag);
      _residual_tag_active[tag] = true;
    }

  _active_residual_tags = active_residual_tags;

  _residual_tag_active.copyToDevice();
}

void
System::setActiveMatrixTags(const std::set<TagID> & tags)
{
  std::set<TagID> active_matrix_tags;

  for (auto tag : tags)
    if (_system.hasMatrix(tag))
    {
      active_matrix_tags.insert(tag);
      _matrix_tag_active[tag] = true;
    }

  _active_matrix_tags = active_matrix_tags;

  _matrix_tag_active.copyToDevice();
}

const libMesh::DofMap &
System::getDofMap() const
{
  return _system.dofMap();
}

} // namespace Moose::Kokkos
