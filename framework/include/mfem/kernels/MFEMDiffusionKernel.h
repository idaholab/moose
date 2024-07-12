#pragma once
#include "MFEMBilinearFormKernel.h"
#include "kernels.h"

class MFEMDiffusionKernel : public MFEMBilinearFormKernel
{
public:
  static InputParameters validParams();

  MFEMDiffusionKernel(const InputParameters & parameters);
  ~MFEMDiffusionKernel() override {}

  std::shared_ptr<platypus::Kernel<mfem::ParBilinearForm>> getKernel() override { return _kernel; }

protected:
  platypus::InputParameters _kernel_params;
  std::shared_ptr<platypus::DiffusionKernel> _kernel{nullptr};
};
