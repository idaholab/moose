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
    _problem(dynamic_cast<platypus::TimeDomainProblem *>(_mfem_problem.mfem_problem.get())),
    _t_step(getParam<Real>("dt")),
    _t_initial(getParam<Real>("start_time")),
    _t_final(getParam<Real>("end_time")),
    _t(_t_initial),
    _it(0),
    _vis_steps(params.get<unsigned int>("visualisation_steps")),
    _last_step(false)
{
  _mfem_problem.transient(true);
}

void
MFEMTransient::Step(double dt, int it) const
{
  // Check if current time step is final
  if (_t + dt >= _t_final - dt / 2)
  {
    _last_step = true;
  }

  // Advance time step.
  _problem->_ode_solver->Step(*(_problem->_f), _t, dt);

  // Sync Host/Device
  _problem->_f->HostRead();

  // Output data
  if (_last_step || (it % _vis_steps) == 0)
  {
    _problem->_outputs.Write(_t);
  }
}

void
MFEMTransient::Solve() const
{
  _it++;
  Step(_t_step, _it);
}

void
MFEMTransient::Execute() const
{
  // Initialise time gridfunctions
  _t = _t_initial;
  _last_step = false;
  _it = 0;
  while (_last_step != true)
  {
    Solve();
  }
}

void
MFEMTransient::init()
{
  _problem = dynamic_cast<platypus::TimeDomainProblem *>(_mfem_problem.mfem_problem.get());
  _mfem_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _mfem_problem.initialSetup();
}

void
MFEMTransient::execute()
{
  _mfem_problem.outputStep(EXEC_INITIAL);
  preExecute();
  Solve();

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