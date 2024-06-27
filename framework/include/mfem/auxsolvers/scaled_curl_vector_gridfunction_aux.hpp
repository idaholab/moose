#pragma once
#include "scaled_vector_gridfunction_aux.hpp"

namespace hephaestus
{

// Scale the curl of a gridfunction in H(Curl) by a scalar Coefficient, and
// store the result. Suitable for solving for H(Div) or H(Curl) conforming
// fields for expressions like E = ρ∇×H
class ScaledCurlVectorGridFunctionAux : public ScaledVectorGridFunctionAux
{
public:
  ScaledCurlVectorGridFunctionAux(
      const std::string & input_gf_name,
      const std::string & scaled_gf_name,
      const std::string & coef_name,
      const double & aConst = 1.0,
      const double & bConst = 1.0,
      const std::string & shift_gf_name = "",
      const hephaestus::InputParameters & solver_options = hephaestus::InputParameters());

  ~ScaledCurlVectorGridFunctionAux() override = default;

  void BuildMixedBilinearForm() override;
};
} // namespace hephaestus
