#pragma once
#include "MFEMLinearFormKernel.h"
#include "kernels.h"

class MFEMVectorFEDomainLFKernel : public MFEMLinearFormKernel
{
public:
  static InputParameters validParams();

  MFEMVectorFEDomainLFKernel(const InputParameters & parameters);
  ~MFEMVectorFEDomainLFKernel() override {}

  std::shared_ptr<platypus::Kernel<mfem::ParLinearForm>> getKernel() override { return _kernel; }

protected:
  platypus::InputParameters _kernel_params;
  std::shared_ptr<platypus::VectorFEDomainLFKernel> _kernel{nullptr};
};
