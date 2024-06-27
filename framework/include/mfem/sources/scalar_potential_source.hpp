#pragma once
#include "source_base.hpp"

namespace hephaestus
{

class ScalarPotentialSource : public hephaestus::Source
{
public:
  ScalarPotentialSource(std::string grad_phi_gf_name,
                        std::string phi_gf_name,
                        std::string hcurl_fespace_name,
                        std::string h1_fespace_name,
                        std::string coef_name,
                        int source_sign = -1,
                        hephaestus::InputParameters solver_options = hephaestus::InputParameters());

  // Override virtual Source destructor to prevent leaks.
  ~ScalarPotentialSource() override;

  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            hephaestus::Coefficients & coefficients) override;
  void Apply(mfem::ParLinearForm * lf) override;
  void SubtractSource(mfem::ParGridFunction * gf) override;
  void BuildH1Diffusion(mfem::Coefficient * Sigma);
  void BuildM1(mfem::Coefficient * Sigma);
  void BuildHCurlMass();
  void BuildWeakDiv();
  void BuildGrad();

  std::string _grad_phi_gf_name;
  std::string _phi_gf_name;
  std::string _hcurl_fespace_name;
  std::string _h1_fespace_name;
  std::string _coef_name;
  int _source_sign;
  const hephaestus::InputParameters _solver_options;

  mfem::ParFiniteElementSpace * _h1_fe_space{nullptr};
  mfem::ParFiniteElementSpace * _h_curl_fe_space{nullptr};
  std::shared_ptr<mfem::ParGridFunction> _phi{nullptr}; // Potential
  hephaestus::BCMap * _bc_map{nullptr};
  mfem::Coefficient * _beta_coef{nullptr};

  std::unique_ptr<mfem::ParBilinearForm> _a0{nullptr};
  std::unique_ptr<mfem::ParBilinearForm> _m1{nullptr};

  mfem::ParBilinearForm * _h_curl_mass;
  mfem::ParMixedBilinearForm * _weak_div{nullptr};

  std::unique_ptr<mfem::ParDiscreteLinearOperator> _grad{nullptr};

  std::unique_ptr<mfem::HypreParMatrix> _diffusion_mat{nullptr};
  std::unique_ptr<mfem::Vector> _p_tdofs{nullptr};
  std::unique_ptr<mfem::Vector> _b0_tdofs{nullptr};

  mutable mfem::HypreSolver * _amg_a0{nullptr};
  mutable std::unique_ptr<hephaestus::DefaultH1PCGSolver> _a0_solver{nullptr};

  std::unique_ptr<mfem::ParLinearForm> _b0{nullptr};
  std::shared_ptr<mfem::ParGridFunction> _grad_phi{nullptr};
  mfem::ParGridFunction * _x_div{nullptr};
  mfem::VectorCoefficient * _source_vec_coef{nullptr};
  mfem::ParGridFunction * _div_free_src_gf{nullptr}; // Source field

  mfem::Solver * _solver{nullptr};
  int _ir_order, _geom;
};

} // namespace hephaestus
