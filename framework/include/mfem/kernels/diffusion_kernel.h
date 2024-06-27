#pragma once
#include "kernel_base.h"

namespace platypus
{

/*
(σ ∇ V, ∇ V')
*/
class DiffusionKernel : public Kernel<mfem::ParBilinearForm>
{
public:
  DiffusionKernel(const platypus::InputParameters & params);

  ~DiffusionKernel() override = default;

  void Init(platypus::GridFunctions & gridfunctions,
            const platypus::FESpaces & fespaces,
            platypus::BCMap & bc_map,
            platypus::Coefficients & coefficients) override;
  void Apply(mfem::ParBilinearForm * blf) override;

  std::string _coef_name;
  mfem::Coefficient * _coef{nullptr};
};

} // namespace platypus
