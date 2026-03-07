//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolverSystem.h"
#include "SolutionInvalidity.h"
#include "FEProblemBase.h"
#include "TimeIntegrator.h"
#include "MooseUtils.h"

using namespace libMesh;

SolverSystem::SolverSystem(SubProblem & subproblem,
                           FEProblemBase & fe_problem,
                           const std::string & name,
                           Moose::VarKindType var_kind)
  : SystemBase(subproblem, fe_problem, name, var_kind),
    _current_solution(nullptr),
    _pc_side(Moose::PCS_DEFAULT),
    _ksp_norm(Moose::KSPN_UNPRECONDITIONED)
{
}

SolverSystem::~SolverSystem() = default;

void
SolverSystem::preInit()
{
  SystemBase::preInit();

  _current_solution = system().current_local_solution.get();

  if (_serialized_solution.get())
    _serialized_solution->init(system().n_dofs(), false, SERIAL);
}

void
SolverSystem::restoreSolutions()
{
  // call parent
  SystemBase::restoreSolutions();
  // and update _current_solution
  _current_solution = system().current_local_solution.get();
}

void
SolverSystem::serializeSolution()
{
  if (_serialized_solution.get())
  {
    if (!_serialized_solution->initialized() || _serialized_solution->size() != system().n_dofs())
    {
      _serialized_solution->clear();
      _serialized_solution->init(system().n_dofs(), false, SERIAL);
    }

    _current_solution->localize(*_serialized_solution);
  }
}

void
SolverSystem::setSolution(const NumericVector<Number> & soln)
{
  _current_solution = &soln;

  auto tag = _subproblem.getVectorTagID(Moose::SOLUTION_TAG);
  associateVectorToTag(const_cast<NumericVector<Number> &>(soln), tag);

  if (_serialized_solution.get())
    serializeSolution();
}

void
SolverSystem::setFixedPointRelaxationFactor(const Real relaxation_factor)
{
  _fixed_point_relaxation_factor = relaxation_factor;
}

void
SolverSystem::clearFixedPointRelaxation()
{
  _fixed_point_relaxation_factor = 1.0;
  _fixed_point_old_solution.reset();
}

void
SolverSystem::saveOldSolutionForFixedPointRelaxation()
{
  if (MooseUtils::absoluteFuzzyEqual(_fixed_point_relaxation_factor, 1.0))
    return;

  if (!_fixed_point_old_solution)
    _fixed_point_old_solution = solution().clone();
  *_fixed_point_old_solution = solution();
}

void
SolverSystem::applyFixedPointRelaxation()
{
  if (MooseUtils::absoluteFuzzyEqual(_fixed_point_relaxation_factor, 1.0))
    return;

  mooseAssert(_fixed_point_old_solution,
              "Fixed point relaxation was requested but the old solution was not saved.");

  auto & sol = solution();
  sol.scale(_fixed_point_relaxation_factor);
  sol.add(1.0 - _fixed_point_relaxation_factor, *_fixed_point_old_solution);
  sol.close();
  update();
}

void
SolverSystem::setPCSide(MooseEnum pcs)
{
  if (pcs == "left")
    _pc_side = Moose::PCS_LEFT;
  else if (pcs == "right")
    _pc_side = Moose::PCS_RIGHT;
  else if (pcs == "symmetric")
    _pc_side = Moose::PCS_SYMMETRIC;
  else if (pcs == "default")
    _pc_side = Moose::PCS_DEFAULT;
  else
    mooseError("Unknown PC side specified.");
}

void
SolverSystem::setMooseKSPNormType(MooseEnum kspnorm)
{
  if (kspnorm == "none")
    _ksp_norm = Moose::KSPN_NONE;
  else if (kspnorm == "preconditioned")
    _ksp_norm = Moose::KSPN_PRECONDITIONED;
  else if (kspnorm == "unpreconditioned")
    _ksp_norm = Moose::KSPN_UNPRECONDITIONED;
  else if (kspnorm == "natural")
    _ksp_norm = Moose::KSPN_NATURAL;
  else if (kspnorm == "default")
    _ksp_norm = Moose::KSPN_DEFAULT;
  else
    mooseError("Unknown ksp norm type specified.");
}

void
SolverSystem::checkInvalidSolution()
{
  auto & solution_invalidity = _app.solutionInvalidity();

  // sync all solution invalid counts to rank 0 process
  solution_invalidity.syncIteration();

  if (solution_invalidity.hasInvalidSolution())
  {
    if (_fe_problem.acceptInvalidSolution())
      if (_fe_problem.showInvalidSolutionConsole())
        solution_invalidity.print(_console);
      else
        mooseWarning("The Solution Invalidity warnings are detected but silenced! "
                     "Use Problem/show_invalid_solution_console=true to show solution counts");
    else
      // output the occurrence of solution invalid in a summary table
      if (_fe_problem.showInvalidSolutionConsole())
        solution_invalidity.print(_console);
  }
}

void
SolverSystem::compute(const ExecFlagType type)
{
  // Let's try not to overcompute
  bool compute_tds = false;
  if (type == EXEC_LINEAR)
    compute_tds = true;
  else if (type == EXEC_NONLINEAR)
  {
    if (_fe_problem.computingScalingJacobian() || matrixFromColoring())
      compute_tds = true;
  }
  else if ((type == EXEC_TIMESTEP_END) || (type == EXEC_FINAL))
  {
    if (_fe_problem.solverParams(number())._type == Moose::ST_LINEAR)
      // We likely don't have a final residual evaluation upon which we compute the time derivatives
      // so we need to do so now
      compute_tds = true;
  }

  // avoid division by dt which might be zero.
  if (compute_tds && _fe_problem.dt() > 0.)
    for (auto & ti : _time_integrators)
    {
      // Do things like compute integration weights
      ti->preStep();
      ti->computeTimeDerivatives();
    }
}
