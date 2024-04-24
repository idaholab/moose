#pragma once
#include "MFEMBilinearFormKernel.h"
#include "kernels.hpp"

class MFEMDiffusionKernel : public MFEMBilinearFormKernel
{
public:
  static InputParameters validParams();

  MFEMDiffusionKernel(const InputParameters & parameters);
  ~MFEMDiffusionKernel() override {}

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  std::shared_ptr<hephaestus::Kernel<mfem::ParBilinearForm>> getKernel() override
  {
    return _kernel;
  }

protected:
  hephaestus::InputParameters _kernel_params;
  std::shared_ptr<hephaestus::DiffusionKernel> _kernel{nullptr};
};
