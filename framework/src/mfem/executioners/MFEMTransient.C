#include "MFEMTransient.h"

registerMooseObject("PlatypusApp", MFEMTransient);

InputParameters
MFEMTransient::validParams()
{
  InputParameters params = MFEMExecutioner::validParams();
  params.addClassDescription("Executioner for transient MFEM problems.");
  params.addParam<Real>("start_time", 0.0, "The start time of the simulation");
  params.addParam<Real>("end_time", 1.0e30, "The end time of the simulation");
  params.addParam<Real>("dt", 1., "The timestep size between solves");
  params.addParam<unsigned int>(
      "visualisation_steps", 1, "The number of timesteps in a transient run");
  return params;
}

MFEMTransient::MFEMTransient(const InputParameters & params)
  : MFEMExecutioner(params),
    _t_step(getParam<Real>("dt")),
    _t_initial(getParam<Real>("start_time")),
    _t_final(getParam<Real>("end_time")),
    _t(_mfem_problem.time()),
    _it(0),
    _vis_steps(params.get<unsigned int>("visualisation_steps")),
    _last_step(false)
{
  _app.setStartTime(_t_initial);
  _t = _t_initial;
  _mfem_problem.transient(true);
}

void
MFEMTransient::constructProblemOperator()
{
  _problem_data._eqn_system = std::make_shared<platypus::TimeDependentEquationSystem>();
  auto problem_operator =
      std::make_unique<platypus::TimeDomainEquationSystemProblemOperator>(_problem_data);
  _problem_operator.reset();
  _problem_operator = std::move(problem_operator);
}

void
MFEMTransient::step(double dt, int) const
{
  // Check if current time step is final
  if (_t + dt >= _t_final - dt / 2)
  {
    _last_step = true;
  }

  // Advance time step.
  _problem_data._ode_solver->Step(_problem_data._f, _t, dt);

  // Synchonise time dependent GridFunctions with updated DoF data.
  _problem_operator->SetTestVariablesFromTrueVectors();

  // Sync Host/Device
  _problem_data._f.HostRead();

  // Execute user objects at timestep end
  _mfem_problem.execute(EXEC_TIMESTEP_END);
  // Perform the output of the current time step
  _mfem_problem.outputStep(EXEC_TIMESTEP_END);
}

void
MFEMTransient::init()
{
  _mfem_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _mfem_problem.initialSetup();

  // Set up initial conditions
  _problem_data._eqn_system->Init(
      _problem_data._gridfunctions,
      _problem_data._fespaces,
      _problem_data._bc_map,
      getParam<MooseEnum>("assembly_level").getEnum<mfem::AssemblyLevel>());

  _problem_operator->SetGridFunctions();
  _problem_operator->Init(_problem_data._f);

  // Set timestepper
  _problem_data._ode_solver = std::make_unique<mfem::BackwardEulerSolver>();
  _problem_data._ode_solver->Init(*(_problem_operator));
  _problem_operator->SetTime(0.0);
}

void
MFEMTransient::execute()
{
  _mfem_problem.outputStep(EXEC_INITIAL);
  preExecute();

  while (_last_step != true)
  {
    _it++;
    step(_t_step, _it);
  }

  _mfem_problem.finishMultiAppStep(EXEC_MULTIAPP_FIXED_POINT_BEGIN,
                                   /*recurse_through_multiapp_levels=*/true);
  _mfem_problem.finishMultiAppStep(EXEC_TIMESTEP_BEGIN, /*recurse_through_multiapp_levels=*/true);
  _mfem_problem.finishMultiAppStep(EXEC_TIMESTEP_END, /*recurse_through_multiapp_levels=*/true);
  _mfem_problem.finishMultiAppStep(EXEC_MULTIAPP_FIXED_POINT_END,
                                   /*recurse_through_multiapp_levels=*/true);

  TIME_SECTION("final", 1, "Executing Final Objects");
  _mfem_problem.execMultiApps(EXEC_FINAL);
  _mfem_problem.finalizeMultiApps();
  _mfem_problem.execute(EXEC_FINAL);
  _mfem_problem.outputStep(EXEC_FINAL);
  _mfem_problem.postExecute();

  postExecute();
}
