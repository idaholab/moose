#pragma once

#include "MFEMCoefficient.h"
#include "MFEMContainers.h"

class MFEMConstantCoefficient : public MFEMCoefficient
{
public:
  static InputParameters validParams();

  MFEMConstantCoefficient(const InputParameters & parameters);
  virtual ~MFEMConstantCoefficient();

  std::shared_ptr<mfem::Coefficient> getCoefficient() const override { return coefficient; }

private:
  std::shared_ptr<mfem::ConstantCoefficient> coefficient{nullptr};
};
