#include "auxsolvers.h"

namespace platypus
{

void
AuxSolvers::Init(const platypus::GridFunctions & gridfunctions, Coefficients & coefficients)
{
  for (const auto & [name, auxsolver] : *this)
  {
    auxsolver->Init(gridfunctions, coefficients);
    _aux_queue.emplace_back(std::pair(auxsolver, name));
  }

  std::sort(_aux_queue.begin(), _aux_queue.end(), AuxSolver::PriorityComparator);
}

void
AuxSolvers::Solve(double t)
{
  for (auto & aux_pair : _aux_queue)
  {
    aux_pair.first->Solve(t);
  }
}

} // namespace platypus
