#pragma once
#include "complex_em_formulation_interface.h"
#include "steady_state_problem_builder.h"

namespace platypus
{

// Specifies output interfaces of a frequency-domain EM formulation.
class FrequencyDomainEMFormulation : public platypus::SteadyStateProblemBuilder,
                                     public platypus::ComplexEMFormulationInterface
{
public:
  FrequencyDomainEMFormulation();
  ~FrequencyDomainEMFormulation() override = default;

protected:
  mfem::ConstantCoefficient * _freq_coef{nullptr};
};
} // namespace platypus
