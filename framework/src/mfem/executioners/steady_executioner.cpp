#include "steady_executioner.hpp"

namespace hephaestus
{

SteadyExecutioner::SteadyExecutioner(const hephaestus::InputParameters & params)
  : Executioner(params), _problem(params.GetParam<hephaestus::SteadyStateProblem *>("Problem"))
{
}

void
SteadyExecutioner::Solve() const
{
  // Advance time step.
  _problem->_preprocessors.Solve();
  _problem->GetOperator()->Solve(*(_problem->_f));
  _problem->_postprocessors.Solve();

  // Output data
  // Output timestep summary to console
  _problem->_outputs.Write();
}

void
SteadyExecutioner::Execute() const
{
  Solve();
}
} // namespace hephaestus
