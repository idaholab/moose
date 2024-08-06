#pragma once
#include "MFEMBilinearFormKernel.h"
#include "kernels.h"

class MFEMCurlCurlKernel : public MFEMBilinearFormKernel
{
public:
  static InputParameters validParams();

  MFEMCurlCurlKernel(const InputParameters & parameters);
  ~MFEMCurlCurlKernel() override {}

  std::shared_ptr<platypus::Kernel<mfem::ParBilinearForm>> getKernel() override { return _kernel; }

protected:
  platypus::InputParameters _kernel_params;
  std::shared_ptr<platypus::CurlCurlKernel> _kernel{nullptr};
};
