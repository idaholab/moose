#include "steady_executioner.h"

namespace platypus
{

SteadyExecutioner::SteadyExecutioner(const platypus::InputParameters & params)
  : Executioner(params), _problem(params.GetParam<platypus::SteadyStateProblem *>("Problem"))
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
} // namespace platypus
