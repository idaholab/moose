#pragma once
#include "../common/pfem_extras.hpp"
#include "inputs.hpp"
#include "statics_formulation.hpp"

namespace hephaestus
{

class MagnetostaticFormulation : public hephaestus::StaticsFormulation
{
public:
  MagnetostaticFormulation(const std::string & magnetic_reluctivity_name,
                           std::string magnetic_permeability_name,
                           const std::string & magnetic_vector_potential_name);

  ~MagnetostaticFormulation() override = default;

  // Enable auxiliary calculation of B ∈ H(div)
  void RegisterMagneticFluxDensityAux(const std::string & b_field_name,
                                      const std::string & external_b_field_name = "") override;

  // Enable auxiliary calculation of H ∈ H(curl)
  void RegisterMagneticFieldAux(const std::string & h_field_name,
                                const std::string & external_h_field_name = "") override;

  // Enable auxiliary calculation of F ∈ L2
  void RegisterLorentzForceDensityAux(const std::string & f_field_name,
                                      const std::string & b_field_name,
                                      const std::string & j_field_name) override;

  void RegisterCoefficients() override;

protected:
  const std::string _magnetic_permeability_name;
  const std::string & _magnetic_reluctivity_name = hephaestus::StaticsFormulation::_alpha_coef_name;
};
} // namespace hephaestus
