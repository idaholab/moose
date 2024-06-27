#pragma once
#include "em_formulation_interface.hpp"
#include "time_domain_equation_system_problem_builder.hpp"

namespace hephaestus
{

// Abstract Factory class of a time-domain EM formulation.
class TimeDomainEMFormulation : public hephaestus::TimeDomainEquationSystemProblemBuilder,
                                public hephaestus::EMFormulationInterface
{
public:
  TimeDomainEMFormulation();
  ~TimeDomainEMFormulation() override = default;
};
} // namespace hephaestus
