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
#include "KokkosQpJacobianOperator.h"

#include "MooseMesh.h"
#include "Assembly.h"
#include "NonlinearSystemBase.h"
#include "LinearSystem.h"
#include "FEProblemBase.h"

#include "libmesh/system.h"
#include "libmesh/fe_interface.h"
#include "libmesh/dof_map.h"

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

  auto * const solution = cast_ptr<PetscVector<Number> *>(sys.current_local_solution.get());

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

  std::set<TagID> dirichlet_tags;

  for (auto bc : nl_system.getKokkosNodalBCWarehouse().getActiveObjects())
  {
    auto nbc = static_cast<NodalBCBase *>(bc.get());

    auto matrix_tags = nbc->getMatrixTags({});

    for (auto tag : matrix_tags)
      getNodalBCDofs(nbc, _nbc_matrix_tag_dof[tag]);

    if (nbc->isDirichletBC())
      for (auto tag : matrix_tags)
        dirichlet_tags.insert(tag);
  }

  // Every DOF a registered Dirichlet-type boundary condition pins -- vertex and bubble (e.g.
  // HIERARCHIC edge/face mode) alike -- comes from one shared device-resident mirror of libMesh's
  // own DOF constraints, rather than each boundary condition's own boundary-node walk above
  if (!dirichlet_tags.empty())
  {
    setupConstraintOperator();

    const auto & mask = _constraint_operator.constrainedMask();

    for (auto tag : dirichlet_tags)
    {
      auto & dofs = _nbc_matrix_tag_dof[tag];

      if (!dofs.isAlloc())
      {
        dofs.create(_num_local_dofs + _num_ghost_dofs);
        dofs = false;
      }

      for (const auto dof : make_range(_num_local_dofs + _num_ghost_dofs))
        if (mask[dof])
          dofs[dof] = true;
    }
  }

  _nbc_matrix_tag_dof.copyToDeviceNested();
}

void
FESystem::getNodalBCDofs(const NodalBCBase * nbc, Array<bool> & dofs)
{
  auto var = nbc->variable().number();
  auto num_dofs = _var_is_vector[var] ? kokkosAssembly().getDimension() : 1;

  if (!dofs.isAlloc())
  {
    dofs.create(_num_local_dofs + _num_ghost_dofs);
    dofs = false;
  }

  for (auto node : nbc->getContiguousNodes())
  {
    auto dof = _local_node_dof_index[var][node];

    if (dof != libMesh::DofObject::invalid_id)
      for (const auto i : make_range(num_dofs))
        dofs[dof + i] = true;
  }

  // Let remote processes know about ghost DOFs associated with nodal BCs not to contribute on them
  syncGhostMask(dofs);
}

void
FESystem::setupConstraintOperator()
{
  auto & nl_system = static_cast<NonlinearSystemBase &>(_system);
  auto & dof_map = _system.system().get_dof_map();

  // ConstraintOperator itself takes every row libMesh's constraint machinery reports with no
  // per-boundary-condition attribution; the one thing it cannot derive from the DofMap alone is
  // whether a row is to be preset (pinned in the solution ahead of the nonlinear solve) or left
  // for the solve to satisfy through its own residual/Jacobian row, which is a property of the
  // registering boundary condition. This walk exists only to answer that one boolean per DOF.
  std::unordered_map<dof_id_type, bool> preset_dofs;
  std::vector<dof_id_type> dof_indices;

  for (auto bc : nl_system.getKokkosNodalBCWarehouse().getActiveObjects())
  {
    auto nbc = static_cast<NodalBCBase *>(bc.get());

    if (!nbc->isDirichletBC())
      continue;

    const auto var = nbc->variable().number();
    const auto preset = nbc->preset();

    for (auto boundary : nbc->boundaryIDs())
      for (auto elem_id : _mesh.getBoundaryActiveSemiLocalElemIds(boundary))
      {
        dof_map.dof_indices(_mesh.elemPtr(elem_id), dof_indices, var);

        for (const auto dof : dof_indices)
          preset_dofs[dof] = preset;
      }
  }

  auto * const solution =
      libMesh::cast_ptr<PetscVector<Number> *>(_system.system().current_local_solution.get());

  _constraint_operator.setup(dof_map, *solution, _num_local_dofs + _num_ghost_dofs, preset_dofs);

  _constrained_solution_tag = _system.subproblem().getVectorTagID(Moose::SOLUTION_TAG);
}

void
FESystem::enableQpJacobianCache()
{
  _qp_jacobian_enabled = true;
  _qp_jacobian.create(kokkosMesh().getNumSubdomains(), _num_vars);
}

void
FESystem::addQpJacobianBlocks(const SubdomainID subdomain,
                              const unsigned int var,
                              const unsigned int blocks)
{
  _qp_jacobian.addBlocks(kokkosMesh().getContiguousSubdomainID(subdomain), var, blocks);
}

void
FESystem::clearQpJacobianCache()
{
  _qp_jacobian.invalidate();
  _qp_jacobian.zero();
}

QpJacobianLevel
FESystem::fineLevel(const TagID matrix_tag) const
{
  return QpJacobianLevel(*this,
                         _var_fe_types,
                         _vectors,
                         _nbc_matrix_tag_dof[matrix_tag],
                         _eliminate_constrained_columns);
}

void
FESystem::applyQpJacobian(const TagID x_tag, const TagID y_tag, const TagID matrix_tag)
{
  QpJacobianOperator(kokkosAssembly(), _qp_jacobian, fineLevel(matrix_tag)).apply(x_tag, y_tag);
}

void
FESystem::computeQpJacobianDiagonal(const TagID diag_tag, const TagID matrix_tag)
{
  QpJacobianOperator(kokkosAssembly(), _qp_jacobian, fineLevel(matrix_tag)).diagonal(diag_tag);
}

void
FESystem::reinit()
{
  // The quadrature-point values kernels and materials read, and the quadrature-point Jacobian cache
  // the matrix-free operator contracts, are both cached from the tagged solution vectors below.
  // Writing each constrained DOF's own constrained value into the solution first is what keeps a
  // free row's equation a function of those values as data rather than of the constrained DOFs as
  // unknowns, which is the residual half of the elimination the operator makes in its trial-space
  // gather; see ConstraintOperator::enforceConstraints(). The two are enabled together, since an
  // operator that eliminates a column the residual still reads would not be its linearization.
  //
  // Only the device-side copy is written: System::sync(DEVICE_TO_HOST) restores a solution vector
  // rather than copying it back, so a constrained row's own residual u_i - g_i still reads the raw
  // iterate when the nodal boundary condition pass evaluates it later. Were that to stop holding,
  // the constrained rows of kokkos/kernels/ad_nonlinear's PetscJacobianTester specs would no longer
  // match their finite difference, since that residual would lose its dependence on its own DOF.
  if (_eliminate_constrained_columns && _constrained_solution_tag != Moose::INVALID_TAG_ID)
    for (const auto tag : _active_solution_tags)
      if (tag == _constrained_solution_tag)
      {
        _constraint_operator.enforceConstraints(getVector(tag));
        break;
      }

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

  if (_qp_jacobian_enabled)
  {
    for (auto subdomain : _mesh.meshSubdomains())
    {
      auto sid = kokkosMesh().getContiguousSubdomainID(subdomain);

      for (const auto var : make_range(_num_vars))
        if (_qp_jacobian.getBlocks(sid, var) != QP_JACOBIAN_NONE)
          _qp_jacobian.createTensors(sid, var, kokkosAssembly().getNumQps(sid));
    }

    _qp_jacobian.copyToDevice();
  }

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
    const auto phi = kokkosAssembly().getVectorPhi(sid, elem_type, fe_type, info.orientation);
    const auto grad_phi =
        kokkosAssembly().getVectorGradPhi(sid, elem_type, fe_type, info.orientation);
    auto jacobian = kokkosAssembly().getJacobian(info, qp).transpose();

    Real3 value = 0;
    Real33 grad = 0;

    for (unsigned int i = 0; i < num_dofs; ++i)
    {
      auto vector = getVectorDofValue(getElemLocalDofIndex(elem, i, var), tag);

      value += vector * phi(i, qp);
      grad += vector * grad_phi(i, qp);
    }

    grad = grad * jacobian;

    getVectorQpVectorValue(info, qp_offset + qp, var, tag) = value;
    getVectorQpVectorGrad(info, qp_offset + qp, var, tag) = grad;
    getVectorQpVectorCurl(info, qp_offset + qp, var, tag) =
        curlFromVectorGradient(grad, kokkosAssembly().getDimension());
  }
  else
  {
    const auto phi = kokkosAssembly().getPhi(sid, elem_type, fe_type, info.orientation);
    const auto grad_phi = kokkosAssembly().getGradPhi(sid, elem_type, fe_type, info.orientation);
    auto jacobian = kokkosAssembly().getJacobian(info, qp);

    Real value = 0;
    Real3 grad = 0;

    for (unsigned int i = 0; i < num_dofs; ++i)
    {
      auto vector = getVectorDofValue(getElemLocalDofIndex(elem, i, var), tag);

      value += vector * phi(i, qp);
      grad += vector * grad_phi(i, qp);
    }

    grad = jacobian * grad;

    getVectorQpValue(info, qp_offset + qp, var, tag) = value;
    getVectorQpGrad(info, qp_offset + qp, var, tag) = grad;
  }
}

} // namespace Moose::Kokkos
