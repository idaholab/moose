#pragma once
#include "kernel_base.hpp"

namespace hephaestus
{

/*
(αv_{n}, ∇×u')
*/
class WeakCurlKernel : public Kernel<mfem::ParLinearForm>
{
public:
  WeakCurlKernel(const hephaestus::InputParameters & params);

  ~WeakCurlKernel() override = default;

  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            hephaestus::Coefficients & coefficients) override;
  void Apply(mfem::ParLinearForm * lf) override;

  std::string _hcurl_gf_name, _hdiv_gf_name;
  std::string _coef_name;
  mfem::ParGridFunction * _u{nullptr};
  mfem::ParGridFunction * _v{nullptr};
  mfem::Coefficient * _coef{nullptr};

  std::unique_ptr<mfem::ParMixedBilinearForm> _weak_curl;
};

} // namespace hephaestus
