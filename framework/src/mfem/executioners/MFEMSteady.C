//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMSteady.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMSteady);

InputParameters
MFEMSteady::validParams()
{
  InputParameters params = MFEMExecutioner::validParams();
  params.addClassDescription("Executioner for steady state MFEM problems.");
  params.addParam<Real>("time", 0.0, "System time");
  return params;
}

MFEMSteady::MFEMSteady(const InputParameters & params)
  : MFEMExecutioner(params),
    _system_time(getParam<Real>("time")),
    _time_step(_mfem_problem.timeStep()),
    _time(_mfem_problem.time()),
    _output_iteration_number(0)
{
  _time = _system_time;
}

void
MFEMSteady::constructProblemOperator()
{
  _problem_data.eqn_system = std::make_shared<Moose::MFEM::EquationSystem>();
  auto problem_operator =
      std::make_unique<Moose::MFEM::EquationSystemProblemOperator>(_problem_data);

  _problem_operator.reset();
  _problem_operator = std::move(problem_operator);
}

void
MFEMSteady::init()
{
  _mfem_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _mfem_problem.initialSetup();

  // Set up initial conditions
  _problem_data.eqn_system->Init(
      _problem_data.gridfunctions,
      _problem_data.fespaces,
      getParam<MooseEnum>("assembly_level").getEnum<mfem::AssemblyLevel>());

  _problem_operator->SetGridFunctions();
  _problem_operator->Init(_problem_data.f);
}

void
MFEMSteady::execute()
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

  // Solve equation system.
  if (_mfem_problem.shouldSolve())
    _problem_operator->Solve(_problem_data.f);

  // Displace mesh, if required
  _mfem_problem.displaceMesh();

  _mfem_problem.computeIndicators();
  _mfem_problem.computeMarkers();

  // need to keep _time in sync with _time_step to get correct output
  _time = _time_step;
  // Execute user objects at timestep end
  _mfem_problem.execute(EXEC_TIMESTEP_END);
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

#endif
