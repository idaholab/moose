//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMSteady.h"
#include "MFEMProblem.h"
#include "EquationSystemProblemOperator.h"

registerMooseMFEMObject("MooseApp", Steady);

namespace Moose::MFEM
{
InputParameters
Steady::validParams()
{
  InputParameters params = ProblemSolve::validParams();
  params += Executioner::validParams();
  params.addClassDescription("Executioner for steady state MFEM problems.");
  params.addParam<Real>("time", 0.0, "System time");
  return params;
}

Steady::Steady(const InputParameters & params)
  : Executioner(params),
    _mfem_problem(dynamic_cast<Problem &>(feProblem())),
    _mfem_problem_data(_mfem_problem.getProblemData()),
    _mfem_problem_solve(*this, getProblemOperators()),
    _system_time(getParam<Real>("time")),
    _time_step(_mfem_problem.timeStep()),
    _time([this]() -> Real & { return this->_mfem_problem.time() = this->_system_time; }()),
    _last_solve_converged(false)
{
  _fixed_point_solve->setInnerSolve(_mfem_problem_solve);

  // If no ProblemOperators have been added by the user, add a default
  if (getProblemOperators().empty())
  {
    if (_mfem_problem.num_type == Problem::NumericType::REAL)
    {
      _mfem_problem_data.eqn_system = std::make_shared<EquationSystem>();
      auto problem_operator = std::make_shared<EquationSystemProblemOperator>(_mfem_problem);
      addProblemOperator(std::move(problem_operator));
    }
    else if (_mfem_problem.num_type == Problem::NumericType::COMPLEX)
    {
      _mfem_problem_data.eqn_system = std::make_shared<ComplexEquationSystem>();
      auto problem_operator = std::make_shared<ComplexEquationSystemProblemOperator>(_mfem_problem);
      addProblemOperator(std::move(problem_operator));
    }
    else
      mooseError("Unknown numeric type. "
                 "Please set the Problem numeric type to either 'real' or 'complex'.");
  }
}

void
Steady::init()
{
  _mfem_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _mfem_problem.initialSetup();
  _fixed_point_solve->initialSetup();

  if (_mfem_problem_data.nonlinear_solver)
    _mfem_problem_data.eqn_system->SetSolverRequiresGradient(
        _mfem_problem_data.nonlinear_solver->requiresGradient());

  // Set up initial conditions
  _mfem_problem_data.eqn_system->Init(
      _mfem_problem_data.gridfunctions,
      _mfem_problem_data.cmplx_gridfunctions,
      getParam<MooseEnum>("assembly_level").getEnum<mfem::AssemblyLevel>());

  for (const auto & problem_operator : getProblemOperators())
  {
    problem_operator->SetGridFunctions();
    problem_operator->Init(_mfem_problem_data.f);
  }
}

void
Steady::execute()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover steady solves!\nExiting...\n" << std::endl;
    return;
  }

  _time_step = 0;
  _time = _time_step;
  _mfem_problem.outputStep(EXEC_INITIAL);
  _time = _system_time;

  preExecute();

  _mfem_problem.advanceState();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;
  _mfem_problem.timestepSetup();

  _last_solve_converged = _fixed_point_solve->solve();

  _mfem_problem.computeIndicators();
  _mfem_problem.computeMarkers();

  // need to keep _time in sync with _time_step to get correct output
  _time = _time_step;
  _mfem_problem.outputStep(EXEC_TIMESTEP_END);
  _time = _system_time;

  {
    TIME_SECTION("final", 1, "Executing Final Objects")
    _mfem_problem.execMultiApps(EXEC_FINAL);
    _mfem_problem.finalizeMultiApps();
    _mfem_problem.postExecute();
    _mfem_problem.execute(EXEC_FINAL);
    _time = _time_step;
    _mfem_problem.outputStep(EXEC_FINAL);
    _time = _system_time;
  }

  postExecute();
}

} // namespace Moose::MFEM
#endif
