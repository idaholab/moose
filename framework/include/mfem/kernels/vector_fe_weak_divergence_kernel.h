#pragma once
#include "kernel_base.h"

namespace platypus
{

/*
(σ u, ∇ V')
*/
class VectorFEWeakDivergenceKernel : public Kernel<mfem::ParMixedBilinearForm>
{
public:
  VectorFEWeakDivergenceKernel(const platypus::InputParameters & params);

  ~VectorFEWeakDivergenceKernel() override = default;

  void Init(platypus::GridFunctions & gridfunctions,
            const platypus::FESpaces & fespaces,
            platypus::BCMap & bc_map,
            platypus::Coefficients & coefficients) override;
  void Apply(mfem::ParMixedBilinearForm * mblf) override;
  std::string _coef_name;
  mfem::Coefficient * _coef{nullptr};
};

} // namespace platypus
