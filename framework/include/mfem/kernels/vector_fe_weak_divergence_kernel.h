#pragma once
#include "kernel_base.h"

namespace hephaestus
{

/*
(σ u, ∇ V')
*/
class VectorFEWeakDivergenceKernel : public Kernel<mfem::ParMixedBilinearForm>
{
public:
  VectorFEWeakDivergenceKernel(const hephaestus::InputParameters & params);

  ~VectorFEWeakDivergenceKernel() override = default;

  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            Coefficients & coefficients) override;
  void Apply(mfem::ParMixedBilinearForm * mblf) override;
  std::string _coef_name;
  mfem::Coefficient * _coef{nullptr};
};

} // namespace hephaestus
