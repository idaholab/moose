//* Solves:
//* ∇×(ν∇×A) = Jᵉ
//*
//* in weak form
//* (ν∇×A, ∇×A') - (Jᵉ, A') - <(ν∇×A)×n, A'>  = 0

//* where:
//* reluctivity ν = 1/μ
//* Magnetic vector potential A
//* Electric field, E = ρJᵉ
//* Magnetic flux density, B = ∇×A
//* Magnetic field H = ν∇×A
//* Current density J = Jᵉ

#include "magnetostatic_formulation.hpp"

#include <utility>

namespace hephaestus
{

MagnetostaticFormulation::MagnetostaticFormulation(
    const std::string & magnetic_reluctivity_name,
    std::string magnetic_permeability_name,
    const std::string & magnetic_vector_potential_name)
  : StaticsFormulation(magnetic_reluctivity_name, magnetic_vector_potential_name),
    _magnetic_permeability_name(std::move(magnetic_permeability_name))
{
}

void
MagnetostaticFormulation::RegisterMagneticFluxDensityAux(const std::string & b_field_name,
                                                         const std::string & external_b_field_name)
{
  //* Magnetic flux density, B = ∇×A
  hephaestus::AuxSolvers & auxsolvers = GetProblem()->_postprocessors;
  auxsolvers.Register(b_field_name,
                      std::make_shared<hephaestus::CurlAuxSolver>(_h_curl_var_name, b_field_name));
}

void
MagnetostaticFormulation::RegisterMagneticFieldAux(const std::string & h_field_name,
                                                   const std::string & external_h_field_name)
{
  //* Magnetic field H = ν∇×A
  hephaestus::AuxSolvers & auxsolvers = GetProblem()->_postprocessors;
  auxsolvers.Register(
      h_field_name,
      std::make_shared<hephaestus::ScaledCurlVectorGridFunctionAux>(_h_curl_var_name,
                                                                    h_field_name,
                                                                    _magnetic_reluctivity_name,
                                                                    1.0,
                                                                    1.0,
                                                                    external_h_field_name));
}

void
MagnetostaticFormulation::RegisterLorentzForceDensityAux(const std::string & f_field_name,
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
MagnetostaticFormulation::RegisterCoefficients()
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
}
} // namespace hephaestus
