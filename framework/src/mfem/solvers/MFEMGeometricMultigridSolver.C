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

registerMooseMFEMObject("MooseApp", GeometricMultigridSolver);

namespace Moose::MFEM
{

InputParameters
GeometricMultigridSolver::validParams()
{
  InputParameters params = LinearSolverBase::validParams();
  params.addClassDescription(
      "Geometric (p-)multigrid preconditioner backed by mfem::GeometricMultigrid. "
      "Requires a Moose::MFEM::FESpaceHierarchy and per-level smoother objects.");

  params.addRequiredParam<std::string>("variable",
                                       "Name of the trial variable this preconditioner acts on.");
  params.addRequiredParam<std::string>(
      "fespace_hierarchy",
      "Name of the Moose::MFEM::FESpaceHierarchy that defines the level structure.");
  params.addRequiredParam<std::vector<std::string>>(
      "smoothers",
      "Names of LinearSolverBase objects used as smoothers on the interior levels "
      "(levels 1 to N-1). May have length 1 (broadcast to all interior levels) or "
      "N-1 (one per interior level, ordered coarse-to-fine).");
  params.addRequiredParam<std::string>("coarse_solver",
                                       "Name of the LinearSolverBase used on the coarsest level.");
  params.addRequiredParam<std::vector<std::string>>(
      "assembly_levels",
      "Assembly level for each level in the hierarchy. Valid values: 'legacy', 'full', "
      "'element', 'partial', 'none'. May have length 1 (broadcast to all N levels) or N.");
  return params;
}

GeometricMultigridSolver::GeometricMultigridSolver(const InputParameters & parameters)
  : LinearSolverBase(parameters),
    _var_name(getParam<std::string>("variable")),
    _smoother_names(getParam<std::vector<std::string>>("smoothers")),
    _coarse_solver_name(getParam<std::string>("coarse_solver"))
{
  // Co-own the hierarchy so it outlives this solver.
  const auto & hierarchy_name = getParam<std::string>("fespace_hierarchy");
  _hierarchy = getMFEMProblem().getProblemData().fespace_hierarchies.GetShared(hierarchy_name);

  // Parse and broadcast assembly levels.
  const int N = _hierarchy->GetNumLevels();
  const auto & asm_strs = getParam<std::vector<std::string>>("assembly_levels");
  const int n_asm = asm_strs.size();
  if (n_asm != 1 && n_asm != N)
    mooseError("GeometricMultigridSolver '",
               name(),
               "': 'assembly_levels' must have length 1 or N = ",
               N,
               " (total levels), got ",
               n_asm,
               ".");

  _assembly_levels.resize(N);
  for (int i = 0; i < N; ++i)
    _assembly_levels[i] = parseAssemblyLevel(n_asm == 1 ? asm_strs[0] : asm_strs[i]);

  constructSolver();
}

void
GeometricMultigridSolver::constructSolver()
{
  // Create the stable proxy that will be CG/GMRES's preconditioner for the
  // entire lifetime of this GMS object.  The actual mfem::GeometricMultigrid
  // is built (or rebuilt) in updateSolver() and the proxy is pointed at it.
  auto proxy = std::make_unique<MGProxy>();
  _mg_proxy = proxy.get(); // non-owning raw pointer for later access
  _solver = std::move(proxy);
}

mfem::AssemblyLevel
GeometricMultigridSolver::parseAssemblyLevel(const std::string & s)
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
  ::mooseError("GeometricMultigridSolver: unknown assembly level '",
               s,
               "'. Valid values: legacy, full, element, partial, none.");
  return mfem::AssemblyLevel::LEGACY;
}

void
GeometricMultigridSolver::updateSolver(mfem::Operator & /*op*/, mfem::Array<int> & /*tdofs*/)
{
  auto & problem = getMFEMProblem();
  auto & pd = problem.getProblemData();

  auto * eq_sys = dynamic_cast<EquationSystem *>(pd.eqn_system.get());
  if (!eq_sys)
    mooseError("GeometricMultigridSolver '",
               name(),
               "': requires a standard (non-complex, non-time-dependent) EquationSystem.");

  if (eq_sys->hasMixedBilinearForms(_var_name))
    mooseError("GeometricMultigridSolver '",
               name(),
               "': mixed bilinear form contributions are not supported for variable '",
               _var_name,
               "'. Block multigrid is required for saddle-point / mixed-field problems.");

  const int N = _hierarchy->GetNumLevels();

  // Validate smoother vector length (levels 1 to N-1 each need a smoother).
  const int n_smooth = _smoother_names.size();
  if (n_smooth != 1 && n_smooth != N - 1)
    mooseError("GeometricMultigridSolver '",
               name(),
               "': 'smoothers' must have length 1 or N-1 = ",
               N - 1,
               ", got ",
               n_smooth,
               ".");

  auto get_smoother = [&](int level) -> LinearSolverBase &
  {
    if (level == 0)
      return problem.getMFEMObject<LinearSolverBase>("Moose::MFEM::SolverBase",
                                                     _coarse_solver_name);
    const std::string & sname = (n_smooth == 1) ? _smoother_names[0] : _smoother_names[level - 1];
    return problem.getMFEMObject<LinearSolverBase>("Moose::MFEM::SolverBase", sname);
  };

  // Obtain essential boundary attribute markers from the equation system.
  mfem::Array<int> ess_bdr = eq_sys->buildEssentialBoundaryMarkers(_var_name);

  // Build new levels' forms; accumulate before touching _mg / _level_*.
  std::vector<std::shared_ptr<mfem::ParBilinearForm>> new_blfs;
  std::vector<std::shared_ptr<mfem::ParNonlinearForm>> new_nlfs;

  auto mg = std::make_unique<mfem::GeometricMultigrid>(*_hierarchy, ess_bdr);
  auto * mg_ptr = mg.get();

  for (int level = 0; level < N; ++level)
  {
    auto & level_fespace =
        static_cast<mfem::ParFiniteElementSpace &>(_hierarchy->GetFESpaceAtLevel(level));

    // Compute essential true DoFs for this level.
    mfem::Array<int> level_tdofs;
    level_fespace.GetEssentialTrueDofs(ess_bdr, level_tdofs);

    // Build level operator.
    mfem::Operator * level_op = nullptr;
    bool own_op = false;

    if (eq_sys->nonlinear())
    {
      // Build a nonlinear form at this level and linearise at zero.
      // Open question #1: the linearisation point should come from restricting
      // the current fine-level solution; zero is used as a placeholder.
      auto nlf =
          eq_sys->buildNonlinearFormForFESpace(_var_name, level_fespace, _assembly_levels[level]);
      mfem::Vector level_x(level_fespace.GetTrueVSize());
      level_x = 0.0;
      level_op = &nlf->GetGradient(level_x);
      // The Jacobian is owned by the nonlinear form; own_op = false.
      own_op = false;
      new_nlfs.push_back(std::move(nlf));
    }
    else
    {
      auto blf =
          eq_sys->buildBilinearFormForFESpace(_var_name, level_fespace, _assembly_levels[level]);

      if (_assembly_levels[level] == mfem::AssemblyLevel::LEGACY)
      {
        // Use the template FormSystemMatrix<HypreParMatrix> overload, which calls
        // HypreParMatrix::MakeRef internally.  The wrapper object is a lightweight
        // view: it does not own the underlying Hypre data, which remains owned by
        // blf (stored in _level_blfs).  The MG is given ownership of the wrapper
        // (own_op=true) and deletes it on destruction without touching the data,
        // matching the idiom used in mfem/examples/ex26p.cpp.
        auto * hyp_mat = new mfem::HypreParMatrix();
        blf->FormSystemMatrix(level_tdofs, *hyp_mat);
        level_op = hyp_mat;
        own_op = true;
      }
      else
      {
        // For non-LEGACY assembly the bilinear form itself acts as the operator.
        level_op = blf.get();
        own_op = false; // owned by new_blfs shared_ptr
      }
      new_blfs.push_back(std::move(blf));
    }

    // Configure the smoother / coarse solver with this level's operator.
    // Each smoother's updateSolver() owns full initialization (including SetOperator).
    auto & level_smoother = get_smoother(level);
    level_smoother.updateSolver(*level_op, level_tdofs);

    mg_ptr->AddLevel(level_op, &level_smoother.getSolver(), own_op, /*ownSmoother=*/false);
  }

  // Atomically replace:
  //  1. Old MG freed (releasing any owned LEGACY matrices).
  //  2. Old forms freed (safe since old MG is already gone).
  //  3. Proxy updated to point at the new MG.
  _mg = std::move(mg);
  _level_blfs = std::move(new_blfs);
  _level_nlfs = std::move(new_nlfs);
  _mg_proxy->setMG(_mg.get());
}

} // namespace Moose::MFEM
#endif
