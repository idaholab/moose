#pragma once

#include "MFEMGeneralUserObject.h"
#include "kernels.h"
#include "gridfunctions.h"

class MFEMBilinearFormKernel : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMBilinearFormKernel(const InputParameters & parameters);
  virtual ~MFEMBilinearFormKernel() {}

  virtual std::shared_ptr<platypus::Kernel<mfem::ParBilinearForm>> getKernel() { return nullptr; }
};
