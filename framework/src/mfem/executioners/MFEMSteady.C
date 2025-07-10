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

registerMooseObject("MooseApp", MFEMSteady);

InputParameters
MFEMSteady::validParams()
{
  InputParameters params = MFEMProblemSolve::validParams();
  params += Executioner::validParams();
  params.addClassDescription("Executioner for steady state MFEM problems.");
  params.addParam<Real>("time", 0.0, "System time");

  params.addParam<std::string>("fe_space", "none", "FE Space to perform p-refinement in");
  return params;
}

MFEMSteady::MFEMSteady(const InputParameters & params)
  : Executioner(params),
    _mfem_problem(dynamic_cast<MFEMProblem &>(feProblem())),
    _mfem_problem_data(_mfem_problem.getProblemData()),
    _mfem_problem_solve(*this, getProblemOperators()),
    _system_time(getParam<Real>("time")),
    _time_step(_mfem_problem.timeStep()),
    _time([this]() -> Real & { return this->_mfem_problem.time() = this->_system_time; }()),
    _last_solve_converged(false)
    _output_iteration_number(0),
    _fe_space_name(getParam<std::string>("fe_space"))
{
  // If no ProblemOperators have been added by the user, add a default
  if (getProblemOperators().empty())
  {
    if (_mfem_problem.num_type == MFEMProblem::NumericType::REAL)
    {
      _mfem_problem_data.eqn_system = std::make_shared<Moose::MFEM::EquationSystem>();
      auto problem_operator =
          std::make_shared<Moose::MFEM::EquationSystemProblemOperator>(_mfem_problem);
      addProblemOperator(std::move(problem_operator));
    }
    else if (_mfem_problem.num_type == MFEMProblem::NumericType::COMPLEX)
    {
      _mfem_problem_data.eqn_system = std::make_shared<Moose::MFEM::ComplexEquationSystem>();
      auto problem_operator =
          std::make_shared<Moose::MFEM::ComplexEquationSystemProblemOperator>(_mfem_problem);
      addProblemOperator(std::move(problem_operator));
    }
    else
      mooseError("Unknown numeric type. "
                 "Please set the Problem numeric type to either 'real' or 'complex'.");
  }
}

void
MFEMSteady::init()
{
  _mfem_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _mfem_problem.initialSetup();

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
MFEMSteady::execute()
{
  // first, we need to set up AMR
  if (_use_amr)
    _problem_operator->SetUpAMR();

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

  _last_solve_converged = _mfem_problem_solve.solve();
  // Solve equation system.
  if (_mfem_problem.shouldSolve())
  {
    _problem_operator->Solve(_problem_data.f);

    bool stop = false;
    bool stop_pref = true;
    bool stop_href = true;
    while (_use_amr and !stop)
    {
      // Check if we have P-Refinement enabled or we've done enough
      // p-refinement steps
      if ( _problem_operator->UsePRefinement() )
      {
        stop_pref = PRefine();
        _problem_operator->Solve(_problem_data.f);
      }

      // Check if we have H-Refinement enabled or we've done enough
      // p-refinement steps
      if ( _problem_operator->UseHRefinement() )
      {
        stop_href = HRefine();
        _problem_operator->Solve(_problem_data.f);
      }

      // Stop when both H_ref and P-ref think it's time to stop
      stop = (stop_href and stop_pref);

      // reset the other two bools
      stop_href = true;
      stop_pref = true;
    }
  }

  // Displace mesh, if required
  _mfem_problem.displaceMesh();

  _mfem_problem.computeIndicators();
  _mfem_problem.computeMarkers();

  // need to keep _time in sync with _time_step to get correct output
  _time = _time_step;
  // Execute user objects at timestep end
  _mfem_problem.execute(EXEC_TIMESTEP_END);

  // Inform objects (e.g aux kernels) that they don't need to update after this point.
  // H/P-refinement sets this to true
  _mfem_problem.setMeshChanged(false);

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

bool
MFEMSteady::addEstimator(std::shared_ptr<MFEMEstimator> estimator)
{
  if (estimator)
  {
    #pragma message "Redundantly setting _use_amr to true twice; pick one place to do it"
    _use_amr = true;
    _problem_operator->AddEstimator(estimator);
    return true;
  }
  else
  {
    return false;
  }
}

bool
MFEMSteady::addRefiner(std::shared_ptr<MFEMThresholdRefiner> refiner)
{
  if (refiner)
  {
    #pragma message "Redundantly setting _use_amr to true twice; pick one place to do it"
    _use_amr = true;
    _problem_operator->AddRefiner(refiner);
    return true;
  }
  else
  {
    return false;
  }
}

bool
MFEMSteady::PRefine()
{
  // Call PRefine in the problem operator
  bool stop = _problem_operator->PRefine();

  UpdateAfterRefinement();

  return stop;
}

bool
MFEMSteady::HRefine()
{
  // Call PRefine in the problem operator
  bool stop = _problem_operator->HRefine();

  UpdateAfterRefinement();

  return stop;
}

void
MFEMSteady::UpdateAfterRefinement()
{
  // Update in the mfem problem
  _mfem_problem.updateAfterRefinement();

  _problem_operator->SetGridFunctions();
}

#endif
