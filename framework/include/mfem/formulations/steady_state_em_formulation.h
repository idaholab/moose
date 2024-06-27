#pragma once
#include "em_formulation_interface.h"
#include "steady_state_problem_builder.h"

namespace hephaestus
{

// Specifies output interfaces of a time-independent problem formulation.
class SteadyStateEMFormulation : public hephaestus::SteadyStateProblemBuilder,
                                 public hephaestus::EMFormulationInterface
{
public:
  SteadyStateEMFormulation();
  ~SteadyStateEMFormulation() override = default;
};
} // namespace hephaestus
