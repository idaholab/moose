//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSystem.h"
#include "KokkosNodalBCBase.h"

#include "MooseMesh.h"
#include "Assembly.h"
#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"

#include "libmesh/system.h"

namespace Moose::Kokkos
{

System::System(SystemBase & system)
  : MeshHolder(*system.mesh().getKokkosMesh()),
    AssemblyHolder(system.feProblem().kokkosAssembly()),
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

  if (dynamic_cast<NonlinearSystemBase *>(&_system))
  {
    setupSparsity();
    checkNodalBCs();
  }

  _qp_solutions.create(MAX_TAG);
  _qp_solutions_grad.create(MAX_TAG);
}

void
System::setupVariables()
{
  auto & sys = _system.system();

  _var_fe_types.create(_num_vars);
  _var_subdomain_active.create(_num_vars, _mesh.nSubdomains());

  for (unsigned int var = 0; var < _num_vars; ++var)
  {
    _var_fe_types[var] = kokkosAssembly().getFETypeID(sys.variable_type(var));

    for (auto subdomain : _mesh.meshSubdomains())
      _var_subdomain_active(var, kokkosMesh().getContiguousSubdomainID(subdomain)) =
          sys.variable(var).active_on_subdomain(subdomain);
  }

  _var_fe_types.copyToDevice();
  _var_subdomain_active.copyToDevice();

  // Set coupling

  auto nl_system = dynamic_cast<NonlinearSystemBase *>(&_system);

  if (nl_system)
  {
    _coupling.create(_num_vars);

    std::map<unsigned int, std::vector<unsigned int>> coupling;

    auto & ce = _system.feProblem().couplingEntries(0, nl_system->number());

    for (const auto & [ivar, jvar] : ce)
      if (ivar->number() != jvar->number())
        coupling[ivar->number()].push_back(jvar->number());

    for (unsigned int var = 0; var < _num_vars; ++var)
      _coupling[var] = coupling[var];

    _coupling.copyToDevice();
  }
}

void
System::setupDofs()
{
  auto & sys = _system.system();

  auto num_elems = kokkosMesh().getNumLocalElements();
  auto num_nodes = kokkosMesh().getContiguousNodeMap().size();

  _residual_tag_active.create(MAX_TAG);
  _residual_tag_active = false;

  _matrix_tag_active.create(MAX_TAG);
  _matrix_tag_active = false;

  _vectors.create(MAX_TAG);
  _matrices.create(MAX_TAG);

  _local_elem_dof_index.create(_num_vars);
  _local_node_dof_index.create(_num_vars);
  _local_to_global_dof_index.create(_num_local_dofs + _num_ghost_dofs);

  _max_dofs_per_elem.create(_num_vars);
  _max_dofs_per_elem = 0;

  auto solution = dynamic_cast<PetscVector<Number> *>(sys.current_local_solution.get());

  std::vector<dof_id_type> dof_indices;

  for (unsigned int var = 0; var < _num_vars; ++var)
  {
    for (auto & [elem, id] : kokkosMesh().getContiguousElementMap())
    {
      _dof_map.dof_indices(elem, dof_indices, var);

      _max_dofs_per_elem[var] =
          std::max(_max_dofs_per_elem[var], static_cast<unsigned int>(dof_indices.size()));
    }

    _local_elem_dof_index[var].create(num_elems, _max_dofs_per_elem[var]);
    _local_elem_dof_index[var] = libMesh::DofObject::invalid_id;

    _local_node_dof_index[var].create(num_nodes);
    _local_node_dof_index[var] = libMesh::DofObject::invalid_id;

    for (auto & [elem, id] : kokkosMesh().getContiguousElementMap())
    {
      _dof_map.dof_indices(elem, dof_indices, var);

      for (unsigned int dof = 0; dof < dof_indices.size(); ++dof)
      {
        _local_elem_dof_index[var](id, dof) = solution->map_global_to_local_index(dof_indices[dof]);
        _local_to_global_dof_index[_local_elem_dof_index[var](id, dof)] = dof_indices[dof];
      }
    }

    for (auto & [node, id] : kokkosMesh().getContiguousNodeMap())
      if (node->processor_id() == _comm.rank())
      {
        _dof_map.dof_indices(node, dof_indices, var);

        if (dof_indices.size())
        {
          for (unsigned int i = 1; i < dof_indices.size(); ++i)
            if (dof_indices[i] != dof_indices[i - 1] + 1)
              mooseError("Kokkos system error: a variable has multiple DOFs on a node, but the DOF "
                         "indices are discontiguous. This is not supported.");

          _local_node_dof_index[var][id] = solution->map_global_to_local_index(dof_indices[0]);
        }
      }
  }

  _local_elem_dof_index.copyToDeviceNested();
  _local_node_dof_index.copyToDeviceNested();
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
  auto pattern = _dof_map.get_sparsity_pattern();

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
System::checkNodalBCs()
{
  auto & nl_system = static_cast<NonlinearSystemBase &>(_system);

  _nbc_residual_tag_dof.create(MAX_TAG);
  _nbc_matrix_tag_dof.create(MAX_TAG);

  for (auto bc : nl_system.getKokkosNodalBCWarehouse().getActiveObjects())
  {
    auto nbc = static_cast<NodalBCBase *>(bc.get());

    auto vector_tags = nbc->getVectorTags({});
    auto matrix_tags = nbc->getMatrixTags({});

    if (_system.feProblem().vectorTagExists("time"))
      vector_tags.insert(_system.feProblem().getVectorTagID("time"));

    if (_system.feProblem().matrixTagExists("time"))
      matrix_tags.insert(_system.feProblem().getMatrixTagID("time"));

    for (auto tag : vector_tags)
      getNodalBCDofs(nbc, _nbc_residual_tag_dof[tag]);

    for (auto tag : matrix_tags)
      getNodalBCDofs(nbc, _nbc_matrix_tag_dof[tag]);
  }

  _nbc_residual_tag_dof.copyToDeviceNested();
  _nbc_matrix_tag_dof.copyToDeviceNested();
}

void
System::getNodalBCDofs(const NodalBCBase * nbc, Array<bool> & dofs)
{
  auto var = nbc->variable().number();

  if (!dofs.isAlloc())
  {
    dofs.create(_num_local_dofs + _num_ghost_dofs);
    dofs = false;
  }

  for (auto node : nbc->getContiguousNodes())
    dofs[_local_node_dof_index[var][node]] = true;

  // Let remote processes know about ghost DOFs associated with nodal BCs not to compute
  // residual on them

  auto num_procs = _comm.size();

  std::vector<std::vector<char>> send(num_procs), recv(num_procs);

  for (processor_id_type proc = 0; proc < num_procs; proc++)
    for (auto dof : _local_comm_list[proc])
      send[proc].push_back(dofs[dof]);

  for (processor_id_type proc = 0; proc < num_procs; proc++)
    _comm.scatter(send, recv[proc], proc);

  for (processor_id_type proc = 0; proc < num_procs; proc++)
    for (dof_id_type i = 0; i < _ghost_comm_list[proc].size(); ++i)
      dofs[_ghost_comm_list[proc][i]] = recv[proc][i];
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
      _vectors[tag] = 0;
    }

    for (auto tag : _active_matrix_tags)
    {
      auto & matrix = _system.getMatrix(tag);

      _matrices[tag].create(matrix, *this);
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
      _matrices[tag].close();
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

void
System::reinit()
{
  for (auto tag : _active_solution_tags)
  {
    if (!_qp_solutions[tag].isAlloc())
      _qp_solutions[tag].create(_mesh.nSubdomains(), _num_vars);

    if (!_qp_solutions_grad[tag].isAlloc())
      _qp_solutions_grad[tag].create(_mesh.nSubdomains(), _num_vars);

    for (auto subdomain : _mesh.meshSubdomains())
    {
      auto sid = kokkosMesh().getContiguousSubdomainID(subdomain);

      for (auto var : _active_variables)
      {
        if (!_var_subdomain_active(var, sid))
          continue;

        if (!_qp_solutions[tag](sid, var).isAlloc())
          _qp_solutions[tag](sid, var).createDevice(kokkosAssembly().getNumQps(sid));

        if (!_qp_solutions_grad[tag](sid, var).isAlloc())
          _qp_solutions_grad[tag](sid, var).createDevice(kokkosAssembly().getNumQps(sid));
      }
    }

    _qp_solutions[tag].copyToDevice();
    _qp_solutions_grad[tag].copyToDevice();
  }

  _qp_solutions.copyToDevice();
  _qp_solutions_grad.copyToDevice();

  dof_id_type num_elems = kokkosMesh().getNumLocalElements();

  _thread.resize({kokkosAssembly().getMaxQpsPerElem(),
                  num_elems,
                  _active_variables.size(),
                  _active_solution_tags.size()});

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>> policy(0, _thread.size());
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();
}

KOKKOS_FUNCTION void
System::operator()(const ThreadID tid) const
{
  auto qp = _thread(tid, 0);
  auto elem = _thread(tid, 1);
  auto var = _active_variables(_thread(tid, 2));
  auto tag = _active_solution_tags(_thread(tid, 3));

  auto info = kokkosMesh().getElementInfo(elem);
  auto sid = info.subdomain;
  auto elem_type = info.type;

  if (!_var_subdomain_active(var, sid))
    return;

  auto fe_type = _var_fe_types[var];
  auto num_dofs = kokkosAssembly().getNumDofs(elem_type, fe_type);
  auto num_qps = kokkosAssembly().getNumQps(info);
  auto qp_offset = kokkosAssembly().getQpOffset(info);

  if (qp >= num_qps)
    return;

  auto & phi = kokkosAssembly().getPhi(sid, elem_type, fe_type);
  auto & grad_phi = kokkosAssembly().getGradPhi(sid, elem_type, fe_type);
  auto jacobian = kokkosAssembly().getJacobian(info, qp);

  Real value = 0;
  Real3 grad = 0;

  for (unsigned int i = 0; i < num_dofs; ++i)
  {
    auto vector = getVectorDofValue(getElemLocalDofIndex(elem, i, var), tag);

    value += vector * phi(i, qp);
    grad += vector * (jacobian * grad_phi(i, qp));
  }

  getVectorQpValue(info, qp_offset + qp, var, tag) = value;
  getVectorQpGrad(info, qp_offset + qp, var, tag) = grad;
}

} // namespace Moose::Kokkos
