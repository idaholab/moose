#include "transient_executioner.h"

namespace platypus
{

TransientExecutioner::TransientExecutioner(const InputParameters & params)
  : Executioner(params),
    _t_step(params.get<float>("TimeStep")),
    _t_initial(params.get<float>("StartTime")),
    _t_final(params.get<float>("EndTime")),
    _t(_t_initial),
    _it(0),
    _vis_steps(params.get<int>("VisualisationSteps")),
    _last_step(false),
    _problem(params.getCheckedPointerParam<platypus::TimeDomainProblem *>("Problem"))
{
}

void
TransientExecutioner::Step(double dt, int it) const
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
TransientExecutioner::Solve() const
{
  _it++;
  Step(_t_step, _it);
}

void
TransientExecutioner::Execute() const
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

} // namespace platypus
