#pragma once
#include "kernel_base.h"

namespace platypus
{

/*
(βu, u')
*/
class VectorFEMassKernel : public Kernel<mfem::ParBilinearForm>
{
public:
  VectorFEMassKernel(const platypus::InputParameters & params);

  ~VectorFEMassKernel() override = default;

  void Init(platypus::GridFunctions & gridfunctions,
            const platypus::FESpaces & fespaces,
            platypus::BCMap & bc_map,
            Coefficients & coefficients) override;
  void Apply(mfem::ParBilinearForm * blf) override;
  std::string _coef_name;
  mfem::Coefficient * _coef{nullptr};
};

} // namespace platypus
