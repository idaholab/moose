//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMGeometricMultigridSolver.h"
#include "MFEMProblem.h"
#include "EquationSystem.h"

registerMooseObject("MooseApp", MFEMGeometricMultigridSolver);

MFEMGeometricMultigridSolver::MGProxy::MGProxy(MFEMGeometricMultigridSolver & owner) : _owner(owner)
{
  // MGProxy is installed as a preconditioner, so Mult() should overwrite its output vector rather
  // than treating it as an initial iterate. This matches MFEM's
  // IterativeSolver::SetPreconditioner() convention, which sets the preconditioner's iterative_mode
  // to false.
  iterative_mode = false;
}

void
MFEMGeometricMultigridSolver::MGProxy::setMG(mfem::GeometricMultigrid & mg)
{
  _mg = &mg;
  height = mg.Height();
  width = mg.Width();
}

void
MFEMGeometricMultigridSolver::MGProxy::SetOperator(const mfem::Operator & op)
{
  _owner.BuildMultigrid(op);
}

void
MFEMGeometricMultigridSolver::MGProxy::Mult(const mfem::Vector & x, mfem::Vector & y) const
{
  MFEM_VERIFY(_mg, "MGProxy: GeometricMultigrid not yet built");
  _mg->Mult(x, y);
}

InputParameters
MFEMGeometricMultigridSolver::validParams()
{
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params.addClassDescription(
      "Geometric (p-)multigrid preconditioner backed by mfem::GeometricMultigrid. "
      "Requires a linear equation system, an MFEMFESpaceHierarchy, and per-level smoother "
      "objects.");

  params.addRequiredParam<std::string>("variable",
                                       "Name of the trial variable this preconditioner acts on.");
  params.addRequiredParam<std::string>(
      "fespace_hierarchy", "Name of the MFEMFESpaceHierarchy that defines the level structure.");
  params.addRequiredParam<std::vector<MFEMSolverName>>(
      "smoothers",
      "Names of LinearSolverBase objects used as smoothers on the interior levels "
      "(levels 1 to N-1). May have length 1 (used on all interior levels) or "
      "N-1 (one per interior level, ordered coarse-to-fine).");
  params.addRequiredParam<MFEMSolverName>(
      "coarse_solver", "Name of the LinearSolverBase used on the coarsest level.");
  params.addRequiredParam<std::vector<std::string>>(
      "assembly_levels",
      "Assembly level for each level in the hierarchy. Valid values: 'legacy', 'full', "
      "'element', 'partial', 'none'. May have length 1 (used on all N levels) or N.");
  return params;
}

MFEMGeometricMultigridSolver::MFEMGeometricMultigridSolver(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters),
    _var_name(getParam<std::string>("variable")),
    _smoother_names(getParam<std::vector<MFEMSolverName>>("smoothers")),
    _coarse_solver_name(getParam<MFEMSolverName>("coarse_solver"))
{
  // Co-own the hierarchy so it outlives this solver.
  const auto & hierarchy_name = getParam<std::string>("fespace_hierarchy");
  _hierarchy = getMFEMProblem().getProblemData().fespace_hierarchies.GetShared(hierarchy_name);

  // Parse assembly levels, optionally expanding a single input value to all levels.
  const int N = _hierarchy->GetNumLevels();
  const auto & asm_strs = getParam<std::vector<std::string>>("assembly_levels");
  const int n_asm = asm_strs.size();
  if (n_asm != 1 && n_asm != N)
    paramError(
        "assembly_levels", "must have length 1 or N = ", N, " (total levels), got ", n_asm, ".");

  _assembly_levels.resize(N);
  for (const auto i : make_range(N))
    _assembly_levels[i] = ParseAssemblyLevel(n_asm == 1 ? asm_strs[0] : asm_strs[i]);

  ConstructSolver();
}

void
MFEMGeometricMultigridSolver::ConstructSolver()
{
  _mg.reset();
  _level_ops.clear();
  _level_blfs.clear();

  auto proxy = std::make_unique<MGProxy>(*this);
  _mg_proxy = proxy.get();
  _solver = std::move(proxy);
}

mfem::AssemblyLevel
MFEMGeometricMultigridSolver::ParseAssemblyLevel(const std::string & s) const
{
  if (s == "legacy")
    return mfem::AssemblyLevel::LEGACY;
  if (s == "full")
    return mfem::AssemblyLevel::FULL;
  if (s == "element")
    return mfem::AssemblyLevel::ELEMENT;
  if (s == "partial")
    return mfem::AssemblyLevel::PARTIAL;
  if (s == "none")
    return mfem::AssemblyLevel::NONE;
  paramError("assembly_levels",
             "unknown assembly level '",
             s,
             "'. Valid values: legacy, full, element, partial, none.");
  return mfem::AssemblyLevel::LEGACY;
}

void
MFEMGeometricMultigridSolver::SetOperator(mfem::Operator & op)
{
  BuildMultigrid(op);
}

void
MFEMGeometricMultigridSolver::BuildMultigrid(const mfem::Operator & op)
{
  auto & problem = getMFEMProblem();
  auto & pd = problem.getProblemData();

  auto * eq_sys = dynamic_cast<Moose::MFEM::EquationSystem *>(pd.eqn_system.get());
  if (!eq_sys)
    mooseError("GeometricMultigridSolver '",
               name(),
               "': requires a standard (non-complex, non-time-dependent) EquationSystem.");

  if (eq_sys->Nonlinear())
    mooseError("GeometricMultigridSolver '",
               name(),
               "': nonlinear equation systems are not currently supported.");

  if (eq_sys->HasMixedBilinearForms(_var_name))
    paramError("variable",
               "mixed bilinear form contributions are not supported for variable '",
               _var_name,
               "'. Block multigrid is required for saddle-point / mixed-field problems.");

  const int N = _hierarchy->GetNumLevels();
  if (N < 1)
    paramError("fespace_hierarchy", "hierarchy must contain at least one level.");
  const int finest_level = N - 1;

  // Validate smoother vector length (levels 1 to N-1 each need a smoother).
  const int n_smooth = _smoother_names.size();
  if (n_smooth != 1 && n_smooth != N - 1)
    paramError("smoothers", "must have length 1 or N-1 = ", N - 1, ", got ", n_smooth, ".");

  auto get_smoother = [&](int level) -> Moose::MFEM::LinearSolverBase &
  {
    if (level == 0)
      return problem.getMFEMObject<Moose::MFEM::LinearSolverBase>("Moose::MFEM::SolverBase",
                                                                  _coarse_solver_name);
    const std::string & sname = (n_smooth == 1) ? _smoother_names[0] : _smoother_names[level - 1];
    return problem.getMFEMObject<Moose::MFEM::LinearSolverBase>("Moose::MFEM::SolverBase", sname);
  };

  // Obtain essential boundary attribute markers from the equation system.
  mfem::Array<int> ess_bdr = eq_sys->BuildEssentialBoundaryMarkers(_var_name);

  auto & finest_fespace =
      static_cast<mfem::ParFiniteElementSpace &>(_hierarchy->GetFESpaceAtLevel(finest_level));
  const int finest_size = finest_fespace.GetTrueVSize();
  if (op.Height() != finest_size || op.Width() != finest_size)
    mooseError("GeometricMultigridSolver '",
               name(),
               "': incoming fine operator has size ",
               op.Height(),
               " x ",
               op.Width(),
               ", but the finest hierarchy space has true size ",
               finest_size,
               ".");

  // Build new levels' forms; accumulate before touching _mg / _level_*.
  std::vector<std::shared_ptr<mfem::ParBilinearForm>> new_blfs;
  std::vector<std::unique_ptr<mfem::OperatorHandle>> new_level_ops;
  new_level_ops.reserve(N - 1);

  auto mg = std::make_unique<mfem::GeometricMultigrid>(*_hierarchy, ess_bdr);
  auto * mg_ptr = mg.get();

  for (const auto level : make_range(N))
  {
    auto & level_fespace =
        static_cast<mfem::ParFiniteElementSpace &>(_hierarchy->GetFESpaceAtLevel(level));

    // Compute essential true DoFs for this level.
    mfem::Array<int> level_tdofs;
    level_fespace.GetEssentialTrueDofs(ess_bdr, level_tdofs);

    // Build level operator.
    mfem::Operator * level_op = nullptr;
    bool own_op = false;

    if (level == finest_level)
    {
      level_op = const_cast<mfem::Operator *>(&op);
      own_op = false;
    }
    else
    {
      auto blf =
          eq_sys->BuildBilinearFormForFESpace(_var_name, level_fespace, _assembly_levels[level]);

      auto level_op_handle = std::make_unique<mfem::OperatorHandle>();
      blf->FormSystemMatrix(level_tdofs, *level_op_handle);
      level_op = level_op_handle->Ptr();
      own_op = false; // owned by level_op_handle or blf
      new_level_ops.push_back(std::move(level_op_handle));
      new_blfs.push_back(std::move(blf));
    }

    // Configure the smoother / coarse solver with this level's operator.
    // Each smoother's SetOperator() owns full initialization.
    auto & level_smoother = get_smoother(level);
    level_smoother.SetOperator(*level_op);

    mg_ptr->AddLevel(level_op, &level_smoother.GetSolver(), own_op, /*ownSmoother=*/false);
  }

  // Atomically replace:
  //  1. Old MG freed, dropping raw pointers into level operators.
  //  2. Old operator handles freed before old forms they may wrap.
  //  3. Proxy updated to point at the new MG and level data.
  _mg = std::move(mg);
  _level_ops = std::move(new_level_ops);
  _level_blfs = std::move(new_blfs);
  _mg_proxy->setMG(*_mg);
}
#endif
