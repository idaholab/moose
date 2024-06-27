#pragma once
#include "auxsolver_base.hpp"

namespace hephaestus
{

class HelmholtzProjector
{
public:
  HelmholtzProjector(const hephaestus::InputParameters & params);

  void Project(hephaestus::GridFunctions & gridfunctions,
               const hephaestus::FESpaces & fespaces,
               hephaestus::BCMap & bc_map);

  void SetForms();
  void SetGrad();
  void SetBCs();
  void SolveLinearSystem();

private:
  std::string _hcurl_fespace_name;
  std::string _h1_fespace_name;
  std::string _gf_grad_name;
  std::string _gf_name;
  hephaestus::InputParameters _solver_options;

  std::shared_ptr<mfem::ParFiniteElementSpace> _h1_fe_space{nullptr};
  mfem::ParFiniteElementSpace * _h_curl_fe_space{nullptr};
  std::shared_ptr<mfem::ParGridFunction> _q{nullptr};

  // H(Curl) projection of user specified source
  std::unique_ptr<mfem::ParGridFunction> _g;

  // Divergence free projected source.
  mfem::ParGridFunction * _div_free_src_gf{nullptr};

  std::unique_ptr<mfem::ParLinearForm> _g_div;
  std::unique_ptr<mfem::ParBilinearForm> _a0;
  std::unique_ptr<mfem::ParMixedBilinearForm> _weak_div;
  std::unique_ptr<mfem::ParDiscreteLinearOperator> _grad;

  mfem::Array<int> _ess_bdr_tdofs;
  hephaestus::BCMap * _bc_map;
};

} // namespace hephaestus
