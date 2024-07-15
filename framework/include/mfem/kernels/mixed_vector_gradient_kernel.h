#pragma once
#include "kernel_base.h"

namespace platypus
{

/*
(σ ∇ V, u')
*/
class MixedVectorGradientKernel : public Kernel<mfem::ParMixedBilinearForm>
{
public:
  MixedVectorGradientKernel(const platypus::InputParameters & params);

  ~MixedVectorGradientKernel() override = default;

  void Init(platypus::GridFunctions & gridfunctions,
            const platypus::FESpaces & fespaces,
            platypus::BCMap & bc_map,
            platypus::Coefficients & coefficients) override;
  void Apply(mfem::ParMixedBilinearForm * mblf) override;
  std::string _coef_name;
  mfem::Coefficient * _coef{nullptr};
};

} // namespace platypus
