#pragma once
#include "em_formulation_interface.h"
#include "steady_state_problem_builder.h"

namespace platypus
{

// Specifies output interfaces of a time-independent problem formulation.
class SteadyStateEMFormulation : public platypus::SteadyStateProblemBuilder,
                                 public platypus::EMFormulationInterface
{
public:
  SteadyStateEMFormulation();
  ~SteadyStateEMFormulation() override = default;
};
} // namespace platypus
