#pragma once
#include "kernel_base.h"

namespace hephaestus
{

/*
(α∇×u, ∇×u')
*/
class CurlCurlKernel : public Kernel<mfem::ParBilinearForm>
{
public:
  CurlCurlKernel(const hephaestus::InputParameters & params);

  ~CurlCurlKernel() override = default;

  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            Coefficients & coefficients) override;
  void Apply(mfem::ParBilinearForm * blf) override;
  std::string _coef_name;
  mfem::Coefficient * _coef{nullptr};
};

} // namespace hephaestus
