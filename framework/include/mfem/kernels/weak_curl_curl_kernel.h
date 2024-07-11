#pragma once
#include "kernel_base.h"

namespace platypus
{

/*
(α∇×u_{n}, ∇×u')
*/
class WeakCurlCurlKernel : public Kernel<mfem::ParLinearForm>
{
public:
  WeakCurlCurlKernel(const platypus::InputParameters & params);

  ~WeakCurlCurlKernel() override = default;

  void Init(platypus::GridFunctions & gridfunctions,
            const platypus::FESpaces & fespaces,
            platypus::BCMap & bc_map,
            platypus::Coefficients & coefficients) override;
  void Apply(mfem::ParLinearForm * lf) override;

  std::string _coupled_gf_name;
  std::string _coef_name;
  mfem::ParGridFunction * _u{nullptr};
  mfem::ParFiniteElementSpace * _h1_fe_space{nullptr};

  mfem::Coefficient * _coef{nullptr};
  std::unique_ptr<mfem::ParBilinearForm> _curl_curl;
};

} // namespace platypus
