#pragma once
#include "auxsolver_base.h"

namespace platypus
{

class HelmholtzProjector
{
public:
  HelmholtzProjector(const platypus::InputParameters & params);

  void Project(platypus::GridFunctions & gridfunctions,
               const platypus::FESpaces & fespaces,
               platypus::BCMap & bc_map);

  void SetForms();
  void SetGrad();
  void SetBCs();
  void SolveLinearSystem();

private:
  std::string _h1_fespace_name;
  std::string _hcurl_fespace_name;
  std::string _gf_grad_name;
  std::string _gf_name;

  std::shared_ptr<mfem::ParFiniteElementSpace> _h1_fe_space{nullptr};
  mfem::ParFiniteElementSpace * _h_curl_fe_space{nullptr};
  std::shared_ptr<mfem::ParGridFunction> _q{nullptr};

  // H(Curl) projection of user specified source
  std::unique_ptr<mfem::ParGridFunction> _g;
  std::unique_ptr<mfem::ParLinearForm> _g_div;
  std::unique_ptr<mfem::ParMixedBilinearForm> _weak_div;
  std::unique_ptr<mfem::ParDiscreteLinearOperator> _grad;
  std::unique_ptr<mfem::ParBilinearForm> _a0;

  // Divergence free projected source.
  mfem::ParGridFunction * _div_free_src_gf{nullptr};

  mfem::Array<int> _ess_bdr_tdofs;
  platypus::BCMap * _bc_map;

  platypus::InputParameters _solver_options;
};

} // namespace platypus
