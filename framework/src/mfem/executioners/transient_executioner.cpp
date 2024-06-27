#include "transient_executioner.hpp"

namespace hephaestus
{

TransientExecutioner::TransientExecutioner(const hephaestus::InputParameters & params)
  : Executioner(params),
    _problem(params.GetParam<hephaestus::TimeDomainProblem *>("Problem")),
    _t_step(params.GetParam<float>("TimeStep")),
    _t_initial(params.GetParam<float>("StartTime")),
    _t_final(params.GetParam<float>("EndTime")),
    _t(_t_initial),
    _it(0),
    _vis_steps(params.GetOptionalParam<int>("VisualisationSteps", 1)),
    _last_step(false)
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
  _problem->_preprocessors.Solve(_t);
  _problem->_ode_solver->Step(*(_problem->_f), _t, dt);
  _problem->_postprocessors.Solve(_t);

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

} // namespace hephaestus
