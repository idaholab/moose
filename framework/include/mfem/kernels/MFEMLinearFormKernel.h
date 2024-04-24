#pragma once

#include "GeneralUserObject.h"
#include "kernels.hpp"
#include "gridfunctions.hpp"

class MFEMLinearFormKernel : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMLinearFormKernel(const InputParameters & parameters);
  virtual ~MFEMLinearFormKernel();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  virtual std::shared_ptr<hephaestus::Kernel<mfem::ParLinearForm>> getKernel() { return nullptr; }
};
