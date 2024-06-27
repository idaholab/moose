#pragma once
#include "source_base.hpp"

namespace hephaestus
{

class DivFreeSource : public hephaestus::Source
{
public:
  DivFreeSource(std::string src_coef_name,
                std::string src_gf_name,
                std::string hcurl_fespace_name,
                std::string h1_fespace_name,
                std::string potential_gf_name = "_source_potential",
                hephaestus::InputParameters solver_options = hephaestus::InputParameters(),
                bool perform_helmholtz_projection = true);

  // Override virtual Source destructor to avoid leaks.
  ~DivFreeSource() override = default;

  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            hephaestus::Coefficients & coefficients) override;
  void Apply(mfem::ParLinearForm * lf) override;
  void SubtractSource(mfem::ParGridFunction * gf) override;
  void BuildHCurlMass();

  std::string _src_gf_name;
  std::string _src_coef_name;
  std::string _potential_gf_name;
  std::string _hcurl_fespace_name;
  std::string _h1_fespace_name;
  const hephaestus::InputParameters _solver_options;
  bool _perform_helmholtz_projection;

  mfem::ParFiniteElementSpace * _h1_fe_space{nullptr};
  mfem::ParFiniteElementSpace * _h_curl_fe_space{nullptr};

  std::shared_ptr<mfem::ParGridFunction> _q; // Potential

  hephaestus::BCMap * _bc_map{nullptr};
  hephaestus::GridFunctions * _gridfunctions{nullptr};
  const hephaestus::FESpaces * _fespaces{nullptr};

  std::unique_ptr<mfem::ParBilinearForm> _h_curl_mass;

  mfem::VectorCoefficient * _source_vec_coef{nullptr};

  // H(Curl) projection of user specified source
  std::shared_ptr<mfem::ParGridFunction> _g;

  // Divergence free projected source
  std::shared_ptr<mfem::ParGridFunction> _div_free_src_gf;

  mfem::Solver * _solver{nullptr};
};

} // namespace hephaestus
