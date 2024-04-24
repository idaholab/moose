#pragma once
#include "MFEMVectorCoefficient.h"

class MFEMVectorFunctionCoefficient : public MFEMVectorCoefficient
{
public:
  static InputParameters validParams();

  MFEMVectorFunctionCoefficient(const InputParameters & parameters);
  virtual ~MFEMVectorFunctionCoefficient();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  std::shared_ptr<mfem::VectorCoefficient> getVectorCoefficient() const override
  {
    return _vector_coefficient;
  }

private:
  const Function & _func;
  std::shared_ptr<mfem::VectorFunctionCoefficient> _vector_coefficient{nullptr};
};
