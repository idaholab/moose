#pragma once
#include "kernel_base.h"

namespace platypus
{

/*
(\\vec f, \\vec u')
*/
class VectorFEDomainLFKernel : public Kernel<mfem::ParLinearForm>
{
public:
  VectorFEDomainLFKernel(const platypus::InputParameters & params);

  ~VectorFEDomainLFKernel() override = default;

  void Init(platypus::GridFunctions & gridfunctions,
            const platypus::FESpaces & fespaces,
            platypus::BCMap & bc_map,
            platypus::Coefficients & coefficients) override;
  void Apply(mfem::ParLinearForm * lf) override;

  std::string _vec_coef_name;
  mfem::VectorCoefficient * _vec_coef{nullptr};
};

} // namespace platypus
