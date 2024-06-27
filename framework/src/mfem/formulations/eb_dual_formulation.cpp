#include "eb_dual_formulation.hpp"

#include <utility>

namespace hephaestus
{

EBDualFormulation::EBDualFormulation(const std::string & magnetic_reluctivity_name,
                                     std::string magnetic_permeability_name,
                                     const std::string & electric_conductivity_name,
                                     const std::string & e_field_name,
                                     const std::string & b_field_name)
  : DualFormulation(
        magnetic_reluctivity_name, electric_conductivity_name, e_field_name, b_field_name),
    _magnetic_permeability_name(std::move(magnetic_permeability_name))
{
}

void
EBDualFormulation::RegisterCurrentDensityAux(const std::string & j_field_name,
                                             const std::string & external_j_field_name)
{
  //* Current density J = Jᵉ + σE
  //* Induced electric field, Jind = σE
  hephaestus::AuxSolvers & auxsolvers = GetProblem()->_postprocessors;
  auxsolvers.Register(
      j_field_name,
      std::make_shared<hephaestus::ScaledVectorGridFunctionAux>(_h_curl_var_name,
                                                                j_field_name,
                                                                _electric_conductivity_name,
                                                                1.0,
                                                                1.0,
                                                                external_j_field_name));
}

void
EBDualFormulation::RegisterLorentzForceDensityAux(const std::string & f_field_name,
                                                  const std::string & b_field_name,
                                                  const std::string & j_field_name)
{
  //* Lorentz force density = J x B
  hephaestus::AuxSolvers & auxsolvers = GetProblem()->_postprocessors;
  auxsolvers.Register(f_field_name,
                      std::make_shared<hephaestus::VectorGridFunctionCrossProductAux>(
                          f_field_name, f_field_name, j_field_name, b_field_name));

  auxsolvers.Get(f_field_name)->SetPriority(2);
}

void
EBDualFormulation::RegisterJouleHeatingDensityAux(const std::string & p_field_name,
                                                  const std::string & e_field_name,
                                                  const std::string & j_field_name)
{
  //* Joule heating density = E.J
  hephaestus::AuxSolvers & auxsolvers = GetProblem()->_postprocessors;
  auxsolvers.Register(p_field_name,
                      std::make_shared<hephaestus::VectorGridFunctionDotProductAux>(
                          p_field_name, p_field_name, "", e_field_name, j_field_name));
  auxsolvers.Get(p_field_name)->SetPriority(2);
}

void
EBDualFormulation::RegisterJouleHeatingDensityAux(const std::string & p_field_name,
                                                  const std::string & e_field_name)
{
  //* Joule heating density = E.J
  hephaestus::AuxSolvers & auxsolvers = GetProblem()->_postprocessors;
  auxsolvers.Register(
      p_field_name,
      std::make_shared<hephaestus::VectorGridFunctionDotProductAux>(
          p_field_name, p_field_name, _electric_conductivity_name, e_field_name, e_field_name));
  auxsolvers.Get(p_field_name)->SetPriority(2);
}

void
EBDualFormulation::RegisterCoefficients()
{
  hephaestus::Coefficients & coefficients = GetProblem()->_coefficients;
  if (!coefficients._scalars.Has(_magnetic_permeability_name))
  {
    MFEM_ABORT(_magnetic_permeability_name + " coefficient not found.");
  }
  coefficients._scalars.Register(
      _magnetic_reluctivity_name,
      std::make_shared<mfem::TransformedCoefficient>(
          &_one_coef, coefficients._scalars.Get(_magnetic_permeability_name), fracFunc));
  DualFormulation::RegisterCoefficients();
}

} // namespace hephaestus
