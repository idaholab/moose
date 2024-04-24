#pragma once
#include "MFEMVectorCoefficient.h"

class MFEMVectorConstantCoefficient : public MFEMVectorCoefficient
{
public:
  static InputParameters validParams();

  MFEMVectorConstantCoefficient(const InputParameters & parameters);
  virtual ~MFEMVectorConstantCoefficient();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  virtual std::shared_ptr<mfem::VectorCoefficient> getVectorCoefficient() const override
  {
    return _vector_coefficient;
  }

private:
  const mfem::Vector _vector;
  std::shared_ptr<mfem::VectorConstantCoefficient> _vector_coefficient{nullptr};
};
