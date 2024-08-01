#pragma once
#include "MFEMBilinearFormKernel.h"
#include "kernels.h"

class MFEMVectorFEMassKernel : public MFEMBilinearFormKernel
{
public:
  static InputParameters validParams();

  MFEMVectorFEMassKernel(const InputParameters & parameters);
  ~MFEMVectorFEMassKernel() override {}

  std::shared_ptr<platypus::Kernel<mfem::ParBilinearForm>> getKernel() override { return _kernel; }

protected:
  platypus::InputParameters _kernel_params;
  std::shared_ptr<platypus::VectorFEMassKernel> _kernel{nullptr};
};
