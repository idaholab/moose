#pragma once
#include "MFEMCoefficient.h"

class MFEMFunctionCoefficient : public MFEMCoefficient
{
public:
  static InputParameters validParams();

  MFEMFunctionCoefficient(const InputParameters & parameters);
  virtual ~MFEMFunctionCoefficient();

  std::shared_ptr<mfem::Coefficient> getCoefficient() const override { return _coefficient; };

private:
  std::shared_ptr<mfem::FunctionCoefficient> _coefficient{nullptr};
};
