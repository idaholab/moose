#pragma once
#include "kernel_base.h"

namespace hephaestus
{

/*
(βu, u')
*/
class VectorFEMassKernel : public Kernel<mfem::ParBilinearForm>
{
public:
  VectorFEMassKernel(const hephaestus::InputParameters & params);

  ~VectorFEMassKernel() override = default;

  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            Coefficients & coefficients) override;
  void Apply(mfem::ParBilinearForm * blf) override;
  std::string _coef_name;
  mfem::Coefficient * _coef{nullptr};
};

} // namespace hephaestus
