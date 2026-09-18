//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RigidBodyContactPredictor.h"

#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "NonlinearSystemBase.h"
#include "SystemBase.h"

#include "libmesh/boundary_info.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/node.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/nonlinear_implicit_system.h"

#include <petscksp.h>

#include <algorithm>
#include <set>
#include <unordered_map>

registerMooseObject("ContactApp", RigidBodyContactPredictor);

InputParameters
RigidBodyContactPredictor::validParams()
{
  InputParameters params = Predictor::validParams();
  params.addClassDescription(
      "Warm-starts the monolithic Newton solve by resolving the contact "
      "subproblem (LM DOFs on the contact sideset + displacement DOFs within "
      "k_hops element hops + optionally the load-control scalar) with a small "
      "sub-Newton iteration.  See uzawa_npc_plan.md.");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "Contact sideset id(s) -- same as the NCP kernel's `boundary`.");
  params.addRequiredParam<VariableName>("lm_variable", "The Lagrange multiplier field variable.");
  params.addRequiredParam<std::vector<VariableName>>(
      "displacements", "Displacement variables in order (x, y[, z]).");
  params.addParam<VariableName>("scalar_variable",
                                "Load-control scalar (RigidBodyLoadControl.variable).  Omit for "
                                "displacement-controlled runs.");
  params.addRangeCheckedParam<unsigned int>(
      "k_hops",
      2,
      "k_hops >= 1",
      "How many element hops beyond the contact sideset to include in the "
      "displacement region.  Bigger = better warm-start, larger sub-problem.");
  params.addRangeCheckedParam<unsigned int>(
      "sub_max_iter", 20, "sub_max_iter >= 1", "Cap on sub-Newton iterations.");
  params.addRangeCheckedParam<Real>(
      "sub_abs_tol", 1e-8, "sub_abs_tol > 0", "Absolute tolerance on the sub-residual norm.");
  params.addRangeCheckedParam<Real>(
      "sub_rel_tol",
      1e-6,
      "sub_rel_tol > 0",
      "Relative tolerance on the sub-residual norm (fraction of initial).");
  // Predictor::validParams() requires `scale`.  Not used here (we do not
  // blend), but exposed to satisfy the base and to disable prediction via
  // `scale = 0` for A/B tests.
  params.set<Real>("scale") = 1.0;
  // Allow MOOSE Controls to flip the predictor on/off at runtime.  Base
  // `MooseObject` already carries the `enable` param; `shouldApply()` below
  // consults `enabled()` so a controllable toggle actually short-circuits.
  params.declareControllable("enable");
  return params;
}

RigidBodyContactPredictor::RigidBodyContactPredictor(const InputParameters & parameters)
  : Predictor(parameters),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary")),
    _lm_var_name(getParam<VariableName>("lm_variable")),
    _disp_var_names(getParam<std::vector<VariableName>>("displacements")),
    _scalar_var_name(isParamValid("scalar_variable")
                         ? static_cast<std::string>(getParam<VariableName>("scalar_variable"))
                         : std::string()),
    _lm_var_num(libMesh::invalid_uint),
    _k_hops(getParam<unsigned int>("k_hops")),
    _sub_max_iter(getParam<unsigned int>("sub_max_iter")),
    _sub_abs_tol(getParam<Real>("sub_abs_tol")),
    _sub_rel_tol(getParam<Real>("sub_rel_tol")),
    _region_is(nullptr),
    _region_ready(false),
    _setup_done(false)
{
}

RigidBodyContactPredictor::~RigidBodyContactPredictor()
{
  if (_region_is)
  {
    auto ierr = ISDestroy(&_region_is);
    (void)ierr;
  }
}

bool
RigidBodyContactPredictor::shouldApply()
{
  if (!enabled())
    return false;
  return Predictor::shouldApply();
}

void
RigidBodyContactPredictor::setupRegion()
{
  if (_setup_done)
    return;
  _setup_done = true;

  // Resolve variable numbers now that MOOSE has instantiated variables.
  _lm_var_num = _fe_problem.getVariable(0, _lm_var_name).number();
  _disp_var_num.resize(_disp_var_names.size());
  for (const auto k : index_range(_disp_var_names))
    _disp_var_num[k] = _fe_problem.getVariable(0, _disp_var_names[k]).number();

  auto & mesh_base = _fe_problem.mesh().getMesh();
  const auto & binfo = mesh_base.get_boundary_info();
  const auto boundary_ids = _fe_problem.mesh().getBoundaryIDs(_boundary_names);
  const std::set<BoundaryID> bset(boundary_ids.begin(), boundary_ids.end());

  // 1) LM nodes: all nodes on the contact boundary.
  std::set<dof_id_type> boundary_node_ids;
  for (const auto & t : binfo.build_node_list())
  {
    const auto node_id = std::get<0>(t);
    const auto bc_id = std::get<1>(t);
    if (bset.count(bc_id))
      boundary_node_ids.insert(node_id);
  }
  if (boundary_node_ids.empty())
  {
    mooseWarning(name(),
                 ": no nodes found on boundary(ies) ",
                 Moose::stringify(_boundary_names),
                 ".  Predictor will be inactive.");
    return;
  }

  // 2) k-hop element-adjacency BFS from those nodes.  Each hop: for each
  //    node in the current frontier, add every node of every element
  //    incident to it.  This spreads the region k elements deep from the
  //    boundary into the bulk.  Nodes are the frontier because element
  //    adjacency via nodes is the widest neighborhood ("touched by") --
  //    weaker (face/edge) adjacency could miss a corner-only neighbor.
  std::unordered_map<dof_id_type, std::vector<dof_id_type>> nodes_to_elem_map;
  libMesh::MeshTools::build_nodes_to_elem_map(mesh_base, nodes_to_elem_map);

  std::set<dof_id_type> region_node_ids = boundary_node_ids;
  std::set<dof_id_type> frontier = boundary_node_ids;
  for (unsigned int hop = 0; hop < _k_hops; ++hop)
  {
    std::set<dof_id_type> next_frontier;
    for (const auto nid : frontier)
    {
      auto it = nodes_to_elem_map.find(nid);
      if (it == nodes_to_elem_map.end())
        continue;
      for (const auto elem_id : it->second)
      {
        const auto * elem = mesh_base.elem_ptr(elem_id);
        if (!elem)
          continue;
        for (unsigned int i = 0; i < elem->n_nodes(); ++i)
        {
          const auto nnid = elem->node_id(i);
          if (region_node_ids.insert(nnid).second)
            next_frontier.insert(nnid);
        }
      }
    }
    frontier.swap(next_frontier);
  }

  // 3) Collect DOFs: LM at boundary nodes + disp at all region nodes +
  //    optional scalar.  Only local DOFs (owned by this rank) go into the
  //    sub-solve -- ghosted DOFs are read-only.
  auto & dof_map = _nl.dofMap();
  const auto first_local = dof_map.first_dof(processor_id());
  const auto end_local = dof_map.end_dof(processor_id());

  auto add_local_dof = [&](dof_id_type d, std::vector<dof_id_type> & sink)
  {
    if (d >= first_local && d < end_local)
      sink.push_back(d);
  };

  std::vector<dof_id_type> tmp;
  for (const auto nid : boundary_node_ids)
  {
    const auto * node = mesh_base.query_node_ptr(nid);
    if (!node)
      continue;
    tmp.clear();
    dof_map.dof_indices(node, tmp, _lm_var_num);
    for (const auto d : tmp)
    {
      add_local_dof(d, _region_dofs);
      add_local_dof(d, _lm_dofs);
    }
  }
  for (const auto nid : region_node_ids)
  {
    const auto * node = mesh_base.query_node_ptr(nid);
    if (!node)
      continue;
    for (const auto var : _disp_var_num)
    {
      tmp.clear();
      dof_map.dof_indices(node, tmp, var);
      for (const auto d : tmp)
        add_local_dof(d, _region_dofs);
    }
  }
  // Scalar DOF: only on the rank that owns it.
  if (!_scalar_var_name.empty())
  {
    const auto & sv = _fe_problem.getScalarVariable(0, _scalar_var_name);
    for (const auto d : sv.dofIndices())
      add_local_dof(d, _region_dofs);
  }

  std::sort(_region_dofs.begin(), _region_dofs.end());
  _region_dofs.erase(std::unique(_region_dofs.begin(), _region_dofs.end()), _region_dofs.end());
  std::sort(_lm_dofs.begin(), _lm_dofs.end());
  _lm_dofs.erase(std::unique(_lm_dofs.begin(), _lm_dofs.end()), _lm_dofs.end());

  // 4) Build PETSc IS.  Global-numbered indices.  We copy so libMesh
  //    ownership of the underlying dof_id_type buffer is not required.
  //    PETSc's PetscInt may differ in size from libMesh's dof_id_type,
  //    so materialize a PetscInt array explicitly.
  std::vector<PetscInt> pi(_region_dofs.size());
  for (const auto k : index_range(_region_dofs))
    pi[k] = static_cast<PetscInt>(_region_dofs[k]);
  LibmeshPetscCallA(_fe_problem.mesh().comm().get(),
                    ISCreateGeneral(_fe_problem.mesh().comm().get(),
                                    cast_int<PetscInt>(pi.size()),
                                    pi.empty() ? nullptr : pi.data(),
                                    PETSC_COPY_VALUES,
                                    &_region_is));
  _region_ready = true;
  _console << name() << ": region has " << _region_dofs.size() << " local DOFs (of which "
           << _lm_dofs.size() << " LM)" << std::endl;
}

void
RigidBodyContactPredictor::apply(NumericVector<Number> & sln)
{
  setupRegion();
  if (!_region_ready)
    return;

  // Save entry state so we can revert on sub-solve failure.
  auto entry_sln = sln.clone();

  // MPI note: `sln` is the SYSTEM's owned-only solution vector.  When
  // MOOSE's `computeResidual` machinery (below) reinits scalar
  // variables, it calls `PetscVector::get()` on the current solution
  // for the scalar's DoF indices -- and for a scalar owned by the
  // last rank, non-owning ranks segfault because that DoF is not local
  // in an owned-only vector.  Work on the system's GHOSTED
  // `current_local_solution` throughout, so scalar reads see valid
  // ghosted values on every rank.
  auto * nl_impl_sys = dynamic_cast<libMesh::NonlinearImplicitSystem *>(&_nl.system());
  if (!nl_impl_sys || !nl_impl_sys->matrix || !nl_impl_sys->current_local_solution)
  {
    mooseWarning(name(), ": nonlinear implicit system storage not allocated; skipping.");
    return;
  }

  // Localize the owned sln into the ghosted work vector.
  nl_impl_sys->update();
  auto & working_sln = *nl_impl_sys->current_local_solution;

  // Scratch residual: clone the ghosted vector so it has matching
  // ghost layout for the tagged residual assembly.
  auto scratch_residual = working_sln.clone();
  auto & residual = *scratch_residual;
  auto & jacobian = *nl_impl_sys->matrix;

  auto * const j_petsc = dynamic_cast<libMesh::PetscMatrix<Number> *>(&jacobian);
  auto * const r_petsc = dynamic_cast<libMesh::PetscVector<Number> *>(&residual);
  auto * const sln_petsc = dynamic_cast<libMesh::PetscVector<Number> *>(&working_sln);
  if (!j_petsc || !r_petsc || !sln_petsc)
  {
    mooseWarning(name(),
                 ": expected PETSc-backed residual/Jacobian/solution vectors; skipping "
                 "predictor.");
    return;
  }
  // Ensure MOOSE knows which nonlinear system this call belongs to --
  // FEProblemBase's associateVectorToTag / associateMatrixToTag dispatch
  // through `_current_nl_sys`, which is only set inside the outer SNES
  // callback under normal use.
  _fe_problem.setCurrentNonlinearSystem(_nl.number());

  Real r_norm_initial = -1.0;
  bool converged = false;
  for (unsigned int iter = 0; iter < _sub_max_iter; ++iter)
  {
    // Sequentially compute R and J.  Attempting to use the combined
    // FEProblemBase::computeResidualAndJacobian(...) here leaves the
    // residual vector empty (its tag-association machinery relies on
    // state set up inside the outer SNES callback that has not yet
    // fired at predictor time).  Separate calls work.
    _fe_problem.computeResidual(working_sln, residual, _nl.number());
    residual.close();
    _fe_problem.computeJacobian(working_sln, jacobian, _nl.number());
    jacobian.close();

    Vec r_sub;
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(),
                      VecGetSubVector(r_petsc->vec(), _region_is, &r_sub));

    PetscReal r_norm;
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), VecNorm(r_sub, NORM_2, &r_norm));

    if (iter == 0)
      r_norm_initial = r_norm;

    if (iter == 0)
      _console << "  " << name() << " |R_sub|[iter 0] = " << r_norm << std::endl;

    if (r_norm < _sub_abs_tol || (r_norm_initial > 0.0 && r_norm < _sub_rel_tol * r_norm_initial))
    {
      converged = true;
      LibmeshPetscCallA(_fe_problem.mesh().comm().get(),
                        VecRestoreSubVector(r_petsc->vec(), _region_is, &r_sub));
      break;
    }

    Mat j_sub;
    LibmeshPetscCallA(
        _fe_problem.mesh().comm().get(),
        MatCreateSubMatrix(j_petsc->mat(), _region_is, _region_is, MAT_INITIAL_MATRIX, &j_sub));

    // Solve j_sub * dx = -r_sub.  Direct LU on the small sub-matrix.
    Vec dx;
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), VecDuplicate(r_sub, &dx));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), VecScale(r_sub, -1.0));

    KSP ksp;
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(),
                      KSPCreate(_fe_problem.mesh().comm().get(), &ksp));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), KSPSetOperators(ksp, j_sub, j_sub));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), KSPSetType(ksp, KSPPREONLY));
    PC pc;
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), KSPGetPC(ksp, &pc));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), PCSetType(pc, PCLU));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), PCFactorSetShiftType(pc, MAT_SHIFT_NONZERO));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), PCFactorSetShiftAmount(pc, 1e-12));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), KSPSolve(ksp, r_sub, dx));
    KSPConvergedReason reason;
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), KSPGetConvergedReason(ksp, &reason));

    if (reason < 0)
    {
      _console << "  " << name() << ": KSP diverged (reason " << reason << ") on iter " << iter
               << ", reverting" << std::endl;
      LibmeshPetscCallA(_fe_problem.mesh().comm().get(), VecDestroy(&dx));
      LibmeshPetscCallA(_fe_problem.mesh().comm().get(), MatDestroy(&j_sub));
      LibmeshPetscCallA(_fe_problem.mesh().comm().get(), KSPDestroy(&ksp));
      LibmeshPetscCallA(_fe_problem.mesh().comm().get(),
                        VecRestoreSubVector(r_petsc->vec(), _region_is, &r_sub));
      converged = false;
      break;
    }

    // Apply update to sln[region_dofs] -- scatter dx into the
    // corresponding entries of the full sln vector.  ADD_VALUES so
    // this is sln <- sln + dx.
    VecScatter scatter;
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(),
                      VecScatterCreate(dx, nullptr, sln_petsc->vec(), _region_is, &scatter));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(),
                      VecScatterBegin(scatter, dx, sln_petsc->vec(), ADD_VALUES, SCATTER_FORWARD));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(),
                      VecScatterEnd(scatter, dx, sln_petsc->vec(), ADD_VALUES, SCATTER_FORWARD));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), VecScatterDestroy(&scatter));

    // Enforce lambda >= 0 by simple projection on all LM DOFs in the
    // region.  Ghosted LM values will be refreshed on the next
    // computeResidualAndJacobian call via setSolution.
    for (const auto d : _lm_dofs)
      if (working_sln(d) < 0.0)
        working_sln.set(d, 0.0);
    working_sln.close();

    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), VecDestroy(&dx));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), MatDestroy(&j_sub));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(), KSPDestroy(&ksp));
    LibmeshPetscCallA(_fe_problem.mesh().comm().get(),
                      VecRestoreSubVector(r_petsc->vec(), _region_is, &r_sub));
  }

  if (!converged)
  {
    _console << name() << ": sub-solve did not converge in " << _sub_max_iter
             << " iters -- reverting to entry state" << std::endl;
    sln = *entry_sln;
    sln.close();
  }
  else
  {
    // Copy the ghosted working solution back into the owned sln.  Only
    // owned entries are valid; ghosts are recomputed on the next
    // sys.update() the outer SNES will do.  Assignment copies all
    // entries but the framework only reads the owned range from sln.
    sln = working_sln;
    sln.close();
    _console << name() << ": sub-solve converged; state warm-started" << std::endl;
  }
}
