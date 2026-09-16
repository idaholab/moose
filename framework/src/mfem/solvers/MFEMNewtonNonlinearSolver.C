//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNewtonNonlinearSolver.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMNewtonNonlinearSolver);

namespace Moose::MFEM
{
void
DampedNewtonSolver::SetOperator(const mfem::Operator & op)
{
  mfem::NewtonSolver::SetOperator(op);

  _ls_x_trial.SetSize(width);
  _ls_x_trial.UseDevice(true);

  _ls_r_trial.SetSize(height);
  _ls_r_trial.UseDevice(true);
}

void
DampedNewtonSolver::SetBacktracking(unsigned int max_its,
                                    mfem::real_t contraction_factor,
                                    mfem::real_t sufficient_decrease)
{
  _ls_max_its = max_its;
  _ls_contraction_factor = contraction_factor;
  _ls_sufficient_decrease = sufficient_decrease;
}

mfem::real_t
DampedNewtonSolver::ComputeScalingFactor(const mfem::Vector & x, const mfem::Vector & b) const
{
  if (_ls_max_its == 0)
    return _damping_factor;

  // NewtonSolver::Mult leaves the residual F(x) - b in r and the Newton update in c.
  const auto have_b = (b.Size() == Height());
  const auto norm = Norm(r);

  auto scale = _damping_factor;
  for (const auto it : make_range(_ls_max_its))
  {
    add(x, -scale, c, _ls_x_trial);
    oper->Mult(_ls_x_trial, _ls_r_trial);
    if (have_b)
      _ls_r_trial -= b;
    const auto trial_norm = Norm(_ls_r_trial);

    if (print_options.iterations)
      mfem::out << "Newton line search iteration " << it << " : damping factor = " << scale
                << ", ||r|| = " << trial_norm << '\n';

    // Sufficient decrease (Armijo) condition on the residual norm, as used by the PETSc
    // backtracking line search SNESLINESEARCHBT.
    if (trial_norm <= (1.0 - _ls_sufficient_decrease * scale) * norm)
      return scale;

    scale *= _ls_contraction_factor;
  }

  if (print_options.warnings)
    mfem::out << "Newton line search: no trial step satisfied the sufficient decrease "
                 "condition!\n";

  // Interrupts the Newton iteration, which then reports no convergence.
  return 0.0;
}
} // namespace Moose::MFEM

InputParameters
MFEMNewtonNonlinearSolver::validParams()
{
  InputParameters params = Moose::MFEM::NonlinearSolverBase::validParams();
  params.addClassDescription("MFEM native nonlinear solver using Newton's method.");
  MooseEnum line_search("none backtracking", "none");
  params.addParam<MooseEnum>(
      "line_search", line_search, "Line search used to shorten the Newton update.");
  params.addRangeCheckedParam<mfem::real_t>(
      "damping_factor",
      1.0,
      "damping_factor > 0 & damping_factor <= 1",
      "Fraction of the full Newton update applied on each "
      "nonlinear iteration, and the first trial step taken when "
      "'line_search' is 'backtracking'.");
  params.addRangeCheckedParam<unsigned int>(
      "line_search_max_its",
      10,
      "line_search_max_its > 0",
      "Maximum number of trial steps taken by the backtracking line search.");
  params.addRangeCheckedParam<mfem::real_t>(
      "line_search_contraction_factor",
      0.5,
      "line_search_contraction_factor > 0 & line_search_contraction_factor < 1",
      "Factor a rejected backtracking trial step is multiplied by to obtain the next one.");
  params.addRangeCheckedParam<mfem::real_t>(
      "line_search_sufficient_decrease",
      1.0e-4,
      "line_search_sufficient_decrease >= 0 & line_search_sufficient_decrease < 1",
      "Coefficient of the sufficient decrease condition the backtracking line search requires "
      "of the residual norm.");
  return params;
}

MFEMNewtonNonlinearSolver::MFEMNewtonNonlinearSolver(const InputParameters & parameters)
  : Moose::MFEM::NonlinearSolverBase(parameters)
{
  ConstructSolver();
}

void
MFEMNewtonNonlinearSolver::ConstructSolver()
{
  auto solver = std::make_unique<Moose::MFEM::DampedNewtonSolver>(getMFEMProblem().getComm());
  solver->iterative_mode = getParam<bool>("use_initial_guess");
  solver->SetRelTol(getParam<mfem::real_t>("rel_tol"));
  solver->SetAbsTol(getParam<mfem::real_t>("abs_tol"));
  solver->SetMaxIter(getParam<unsigned int>("max_its"));
  solver->SetPrintLevel(getParam<unsigned int>("print_level"));
  solver->SetDampingFactor(getParam<mfem::real_t>("damping_factor"));

  if (getParam<MooseEnum>("line_search") == "backtracking")
    solver->SetBacktracking(getParam<unsigned int>("line_search_max_its"),
                            getParam<mfem::real_t>("line_search_contraction_factor"),
                            getParam<mfem::real_t>("line_search_sufficient_decrease"));
  else
    for (const auto & param : {"line_search_max_its",
                               "line_search_contraction_factor",
                               "line_search_sufficient_decrease"})
      if (isParamSetByUser(param))
        paramError(param, "Only used when 'line_search' is set to 'backtracking'.");

  _solver = std::move(solver);
}

void
MFEMNewtonNonlinearSolver::SetLinearSolver(mfem::Solver & solver)
{
  cast_ref<mfem::NewtonSolver &>(GetSolver()).SetSolver(solver);
}
#endif
