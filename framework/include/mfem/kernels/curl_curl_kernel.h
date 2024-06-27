#pragma once
#include "kernel_base.h"

namespace platypus
{

/*
(α∇×u, ∇×u')
*/
class CurlCurlKernel : public Kernel<mfem::ParBilinearForm>
{
public:
  CurlCurlKernel(const platypus::InputParameters & params);

  ~CurlCurlKernel() override = default;

  void Init(platypus::GridFunctions & gridfunctions,
            const platypus::FESpaces & fespaces,
            platypus::BCMap & bc_map,
            Coefficients & coefficients) override;
  void Apply(mfem::ParBilinearForm * blf) override;
  std::string _coef_name;
  mfem::Coefficient * _coef{nullptr};
};

} // namespace platypus
