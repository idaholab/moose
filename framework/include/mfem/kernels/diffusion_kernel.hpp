#pragma once
#include "kernel_base.hpp"

namespace hephaestus
{

/*
(σ ∇ V, ∇ V')
*/
class DiffusionKernel : public Kernel<mfem::ParBilinearForm>
{
public:
  DiffusionKernel(const hephaestus::InputParameters & params);

  ~DiffusionKernel() override = default;

  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            hephaestus::Coefficients & coefficients) override;
  void Apply(mfem::ParBilinearForm * blf) override;

  std::string _coef_name;
  mfem::Coefficient * _coef{nullptr};
};

} // namespace hephaestus
