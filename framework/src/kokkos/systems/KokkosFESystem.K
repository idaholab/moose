//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFESystem.h"
#include "KokkosNodalBCBase.h"

#include "MooseMesh.h"
#include "Assembly.h"
#include "NonlinearSystemBase.h"
#include "LinearSystem.h"
#include "FEProblemBase.h"

#include "libmesh/system.h"
#include "libmesh/fe_interface.h"

namespace Moose::Kokkos
{

FESystem::FESystem(SystemBase & system)
  : System(system), AssemblyHolder(system.feProblem().kokkosAssembly())
{
  setupVariables();
  setupDofs();
  setupCoupling();

  if (dynamic_cast<NonlinearSystemBase *>(&_system))
    setupNodalBCDofs();

  _qp_solutions.create(MAX_TAG);
  _qp_solutions_grad.create(MAX_TAG);
  _qp_vector_solutions.create(MAX_TAG);
  _qp_vector_solutions_grad.create(MAX_TAG);
  _qp_vector_solutions_curl.create(MAX_TAG);
}

FESystem::FESystem(System & base, SystemBase & system)
  : System(base), AssemblyHolder(system.feProblem().kokkosAssembly())
{
  setupVariables();
  setupDofs();

  if (dynamic_cast<NonlinearSystemBase *>(&_system))
    setupNodalBCDofs();

  _qp_solutions.create(MAX_TAG);
  _qp_solutions_grad.create(MAX_TAG);
  _qp_vector_solutions.create(MAX_TAG);
  _qp_vector_solutions_grad.create(MAX_TAG);
  _qp_vector_solutions_curl.create(MAX_TAG);
}

void
FESystem::setupVariables()
{
  auto & sys = _system.system();

  _var_fe_types.create(_num_vars);
  _var_is_vector.create(_num_vars);

  for (unsigned int var = 0; var < _num_vars; ++var)
  {
    const auto fe_type = sys.variable_type(var);

    _var_fe_types[var] = kokkosAssembly().getFETypeID(fe_type);
    _var_is_vector[var] = FEInterface::field_type(fe_type) == libMesh::TYPE_VECTOR;
  }

  _var_fe_types.copyToDevice();
  _var_is_vector.copyToDevice();
}

void
FESystem::setupCoupling()
{
  if (auto * const nl_system = dynamic_cast<NonlinearSystemBase *>(&_system))
  {
    _coupling.create(_num_vars);

    std::map<unsigned int, std::vector<unsigned int>> coupling;

    auto & ce = _system.feProblem().couplingEntries(0, nl_system->number());

    for (const auto & [ivar, jvar] : ce)
      if (ivar->number() != jvar->number())
        coupling[ivar->number()].push_back(jvar->number());

    for (const auto var : make_range(_num_vars))
      _coupling[var] = coupling[var];

    _coupling.copyToDevice();
  }
}

void
FESystem::setupDofs()
{
  auto & sys = _system.system();

  auto num_nodes = kokkosMesh().getNumLocalNodes();

  _local_node_dof_index.create(_num_vars);

  auto * const solution =
      libMesh::cast_ptr<PetscVector<Number> *>(sys.current_local_solution.get());

#ifdef MOOSE_ENABLE_KOKKOS_GPU
  // Kokkos array thinks OpenMP clause is device code when using OpenMP backend
#pragma omp parallel for
#endif
  for (unsigned int var = 0; var < _num_vars; ++var)
  {
    std::vector<dof_id_type> dof_indices;

    _local_node_dof_index[var].create(num_nodes);
    _local_node_dof_index[var] = libMesh::DofObject::invalid_id;

    for (const auto node : kokkosMesh().getLocalNodes())
      if (node->processor_id() == _comm.rank())
      {
        const auto id = kokkosMesh().getContiguousNodeID(node);

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

  _local_node_dof_index.copyToDeviceNested();
}

void
FESystem::setupNodalBCDofs()
{
  auto & nl_system = static_cast<NonlinearSystemBase &>(_system);

  _nbc_matrix_tag_dof.create(MAX_TAG);

  for (auto bc : nl_system.getKokkosNodalBCWarehouse().getActiveObjects())
  {
    auto nbc = static_cast<NodalBCBase *>(bc.get());

    auto matrix_tags = nbc->getMatrixTags({});

    for (auto tag : matrix_tags)
      getNodalBCDofs(nbc, _nbc_matrix_tag_dof[tag]);
  }

  _nbc_matrix_tag_dof.copyToDeviceNested();
}

void
FESystem::getNodalBCDofs(const NodalBCBase * nbc, Array<bool> & dofs)
{
  auto var = nbc->variable().number();

  if (!dofs.isAlloc())
  {
    dofs.create(_num_local_dofs + _num_ghost_dofs);
    dofs = false;
  }

  for (auto node : nbc->getContiguousNodes())
  {
    auto dof = _local_node_dof_index[var][node];

    if (dof != libMesh::DofObject::invalid_id)
      dofs[dof] = true;
  }

  // Let remote processes know about ghost DOFs associated with nodal BCs not to contribute on them

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
FESystem::reinit()
{
  for (auto tag : _active_solution_tags)
  {
    if (!_qp_solutions[tag].isAlloc())
      _qp_solutions[tag].create(kokkosMesh().getNumSubdomains(), _num_vars);

    if (!_qp_solutions_grad[tag].isAlloc())
      _qp_solutions_grad[tag].create(kokkosMesh().getNumSubdomains(), _num_vars);

    if (!_qp_vector_solutions[tag].isAlloc())
      _qp_vector_solutions[tag].create(kokkosMesh().getNumSubdomains(), _num_vars);

    if (!_qp_vector_solutions_grad[tag].isAlloc())
      _qp_vector_solutions_grad[tag].create(kokkosMesh().getNumSubdomains(), _num_vars);

    if (!_qp_vector_solutions_curl[tag].isAlloc())
      _qp_vector_solutions_curl[tag].create(kokkosMesh().getNumSubdomains(), _num_vars);

    for (auto subdomain : _mesh.meshSubdomains())
    {
      auto sid = kokkosMesh().getContiguousSubdomainID(subdomain);

      for (auto var : _active_variables)
      {
        if (!_var_subdomain_active(var, sid))
          continue;

        if (_var_is_vector[var])
        {
          if (!_qp_vector_solutions[tag](sid, var).isAlloc())
            _qp_vector_solutions[tag](sid, var).createDevice(kokkosAssembly().getNumQps(sid));

          if (!_qp_vector_solutions_grad[tag](sid, var).isAlloc())
            _qp_vector_solutions_grad[tag](sid, var).createDevice(kokkosAssembly().getNumQps(sid));

          if (!_qp_vector_solutions_curl[tag](sid, var).isAlloc())
            _qp_vector_solutions_curl[tag](sid, var).createDevice(kokkosAssembly().getNumQps(sid));
        }
        else
        {
          if (!_qp_solutions[tag](sid, var).isAlloc())
            _qp_solutions[tag](sid, var).createDevice(kokkosAssembly().getNumQps(sid));

          if (!_qp_solutions_grad[tag](sid, var).isAlloc())
            _qp_solutions_grad[tag](sid, var).createDevice(kokkosAssembly().getNumQps(sid));
        }
      }
    }

    _qp_solutions[tag].copyToDevice();
    _qp_solutions_grad[tag].copyToDevice();
    _qp_vector_solutions[tag].copyToDevice();
    _qp_vector_solutions_grad[tag].copyToDevice();
    _qp_vector_solutions_curl[tag].copyToDevice();
  }

  _qp_solutions.copyToDevice();
  _qp_solutions_grad.copyToDevice();
  _qp_vector_solutions.copyToDevice();
  _qp_vector_solutions_grad.copyToDevice();
  _qp_vector_solutions_curl.copyToDevice();

  dof_id_type num_elems = kokkosMesh().getNumLocalElements();

  _thread.resize(kokkosAssembly().getMaxQpsPerElem(),
                 num_elems,
                 _active_variables.size(),
                 _active_solution_tags.size());

  ::Kokkos::RangePolicy<ExecSpace, ::Kokkos::IndexType<ThreadID>> policy(0, _thread.size());
  ::Kokkos::parallel_for(policy, *this);
  ::Kokkos::fence();
}

KOKKOS_FUNCTION void
FESystem::operator()(const ThreadID tid) const
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

  if (_var_is_vector[var])
  {
    auto & phi = kokkosAssembly().getVectorPhi(sid, elem_type, fe_type);
    auto & grad_phi = kokkosAssembly().getVectorGradPhi(sid, elem_type, fe_type);
    auto jacobian = kokkosAssembly().getJacobian(info, qp).transpose();

    Real3 value = 0;
    Real33 grad = 0;

    for (unsigned int i = 0; i < num_dofs; ++i)
    {
      auto vector = getVectorDofValue(getElemLocalDofIndex(elem, i, var), tag);

      value += vector * phi(i, qp);
      grad += vector * (grad_phi(i, qp) * jacobian);
    }

    getVectorQpVectorValue(info, qp_offset + qp, var, tag) = value;
    getVectorQpVectorGrad(info, qp_offset + qp, var, tag) = grad;
    getVectorQpVectorCurl(info, qp_offset + qp, var, tag) =
        curlFromVectorGradient(grad, kokkosAssembly().getDimension());
  }
  else
  {
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
}

} // namespace Moose::Kokkos
