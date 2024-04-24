#pragma once

#include "MFEMCoefficient.h"
#include "gridfunctions.hpp"

class MFEMConstantCoefficient : public MFEMCoefficient
{
public:
  static InputParameters validParams();

  MFEMConstantCoefficient(const InputParameters & parameters);
  virtual ~MFEMConstantCoefficient();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  std::shared_ptr<mfem::Coefficient> getCoefficient() const override { return coefficient; }

private:
  std::shared_ptr<mfem::ConstantCoefficient> coefficient{nullptr};
};
