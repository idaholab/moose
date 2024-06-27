#pragma once
#include "em_formulation_interface.h"
#include "time_domain_equation_system_problem_builder.h"

namespace platypus
{

// Abstract Factory class of a time-domain EM formulation.
class TimeDomainEMFormulation : public platypus::TimeDomainEquationSystemProblemBuilder,
                                public platypus::EMFormulationInterface
{
public:
  TimeDomainEMFormulation();
  ~TimeDomainEMFormulation() override = default;
};
} // namespace platypus
