#pragma once
#include "frequency_domain_em_formulation.hpp"

namespace hephaestus
{
/*
Formulation for solving:
∇×(α∇×u) + iωβu - ω²γu = g

via the weak form:
(α∇×u, ∇×u') + (iωβu, u') - (ω²ζu, u') - <(α∇×u)×n, u'> = (g, u')

where
u ∈ H(curl)
g ∈ H(div) is a divergence-free source field
ω is the angular frequency
α is the stiffness coefficient
ωβ is the loss coefficient
ω²γ is the mass coefficient

Dirichlet boundaries strongly constrain n×n×u
Integrated boundaries weakly constrain (α∇×u)×n
Robin boundaries weakly constrain (α∇×u)×n + γ(n×n×u) = F

Divergence cleaning (such as via Helmholtz projection)
should be performed on g before use in this operator.
*/
class ComplexMaxwellFormulation : public hephaestus::FrequencyDomainEMFormulation
{
public:
  ComplexMaxwellFormulation(std::string frequency_coef_name,
                            std::string alpha_coef_name,
                            std::string beta_coef_name,
                            std::string zeta_coef_name,
                            std::string h_curl_var_complex_name,
                            std::string h_curl_var_real_name,
                            std::string h_curl_var_imag_name);

  ~ComplexMaxwellFormulation() override = default;

  void ConstructJacobianSolver() override;

  void ConstructOperator() override;

  void RegisterGridFunctions() override;

  void RegisterCoefficients() override;

  // std::vector<mfem::ParGridFunction *> local_trial_vars, local_test_vars;
protected:
  const std::string _alpha_coef_name;
  const std::string _beta_coef_name;
  const std::string _zeta_coef_name;
  const std::string _frequency_coef_name;
  const std::string _h_curl_var_complex_name;
  const std::string _h_curl_var_real_name;
  const std::string _h_curl_var_imag_name;
  const std::string _mass_coef_name;
  const std::string _loss_coef_name;
};

class ComplexMaxwellOperator : public ProblemOperator
{
public:
  ComplexMaxwellOperator(hephaestus::Problem & problem,
                         std::string h_curl_var_complex_name,
                         std::string h_curl_var_real_name,
                         std::string h_curl_var_imag_name,
                         std::string stiffness_coef_name,
                         std::string mass_coef_name,
                         std::string loss_coef_name);

  ~ComplexMaxwellOperator() override = default;

  void SetGridFunctions() override;
  void Init(mfem::Vector & X) override;
  void Solve(mfem::Vector & X) override;

  std::string _h_curl_var_complex_name, _h_curl_var_real_name, _h_curl_var_imag_name,
      _stiffness_coef_name, _mass_coef_name, _loss_coef_name;

  mfem::ComplexOperator::Convention _conv{mfem::ComplexOperator::HERMITIAN};

  std::unique_ptr<mfem::ParComplexGridFunction> _u{nullptr};
  mfem::Coefficient * _stiff_coef{nullptr}; // Dia/Paramagnetic Material Coefficient
  mfem::Coefficient * _mass_coef{nullptr};  // -omega^2 epsilon
  mfem::Coefficient * _loss_coef{nullptr};  // omega sigma

  mfem::Array<int> _ess_bdr_tdofs;
};

} // namespace hephaestus
