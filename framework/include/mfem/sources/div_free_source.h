#pragma once
#include "source_base.h"

namespace platypus
{

class DivFreeSource : public platypus::Source
{
public:
  DivFreeSource(std::string src_coef_name,
                std::string src_gf_name,
                std::string hcurl_fespace_name,
                std::string h1_fespace_name,
                std::string potential_gf_name = "_source_potential",
                platypus::InputParameters solver_options = platypus::InputParameters(),
                bool perform_helmholtz_projection = true);

  // Override virtual Source destructor to avoid leaks.
  ~DivFreeSource() override = default;

  void Init(platypus::GridFunctions & gridfunctions,
            const platypus::FESpaces & fespaces,
            platypus::BCMap & bc_map,
            Coefficients & coefficients) override;
  void Apply(mfem::ParLinearForm * lf) override;
  void SubtractSource(mfem::ParGridFunction * gf) override;
  void BuildHCurlMass();

  std::string _src_coef_name;
  std::string _src_gf_name;
  std::string _hcurl_fespace_name;
  std::string _h1_fespace_name;
  std::string _potential_gf_name;
  const platypus::InputParameters _solver_options;
  bool _perform_helmholtz_projection;
  std::unique_ptr<mfem::ParBilinearForm> _h_curl_mass;

  mfem::ParFiniteElementSpace * _h1_fe_space{nullptr};
  mfem::ParFiniteElementSpace * _h_curl_fe_space{nullptr};

  std::shared_ptr<mfem::ParGridFunction> _q; // Potential

  platypus::BCMap * _bc_map{nullptr};
  platypus::GridFunctions * _gridfunctions{nullptr};
  const platypus::FESpaces * _fespaces{nullptr};

  mfem::VectorCoefficient * _source_vec_coef{nullptr};

  // H(Curl) projection of user specified source
  std::shared_ptr<mfem::ParGridFunction> _g;

  // Divergence free projected source
  std::shared_ptr<mfem::ParGridFunction> _div_free_src_gf;

  mfem::Solver * _solver{nullptr};
};

} // namespace platypus
