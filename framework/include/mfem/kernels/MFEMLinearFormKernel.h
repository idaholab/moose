#pragma once

#include "MFEMGeneralUserObject.h"
#include "kernels.h"
#include "gridfunctions.h"

class MFEMLinearFormKernel : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMLinearFormKernel(const InputParameters & parameters);
  virtual ~MFEMLinearFormKernel();

  virtual std::shared_ptr<platypus::Kernel<mfem::ParLinearForm>> getKernel() { return nullptr; }
};
