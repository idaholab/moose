#pragma once
#include "../common/pfem_extras.hpp"
#include "hcurl_formulation.hpp"
#include "inputs.hpp"

namespace hephaestus
{

class HFormulation : public hephaestus::HCurlFormulation
{
public:
  HFormulation(const std::string & electric_resistivity_name,
               std::string electric_conductivity_name,
               const std::string & magnetic_permeability_name,
               const std::string & h_field_name);

  ~HFormulation() override = default;

  // Enable auxiliary calculation of J ∈ H(div)
  void RegisterCurrentDensityAux(const std::string & j_field_name,
                                 const std::string & external_j_field_name = "") override;

  // Enable auxiliary calculation of B ∈ H(div)
  void RegisterMagneticFluxDensityAux(const std::string & b_field_name,
                                      const std::string & external_b_field_name = "") override;

  // Enable auxiliary calculation of E ∈ H(curl)
  void RegisterElectricFieldAux(const std::string & e_field_name,
                                const std::string & external_e_field_name = "") override;

  // Enable auxiliary calculation of H ∈ H(curl)
  void RegisterMagneticFieldAux(const std::string & h_field_name,
                                const std::string & external_h_field_name = "") override;

  // Enable auxiliary calculation of F ∈ L2
  void RegisterLorentzForceDensityAux(const std::string & f_field_name,
                                      const std::string & b_field_name,
                                      const std::string & j_field_name) override;

  // Enable auxiliary calculation of P ∈ L2
  void RegisterJouleHeatingDensityAux(const std::string & p_field_name,
                                      const std::string & e_field_name,
                                      const std::string & j_field_name) override;
  void RegisterJouleHeatingDensityAux(const std::string & p_field_name,
                                      const std::string & e_field_name) override;

  void RegisterCoefficients() override;

protected:
  const std::string _electric_conductivity_name;
  const std::string & _electric_resistivity_name = hephaestus::HCurlFormulation::_alpha_coef_name;
  const std::string & _magnetic_permeability_name = hephaestus::HCurlFormulation::_beta_coef_name;
};
} // namespace hephaestus
