#pragma once
#include "MFEMCoefficient.h"
#include "PlatypusUtils.h"
#include "auxsolvers.hpp"

class MFEMVariableDependentFunctionCoefficient : public MFEMCoefficient,
                                                 public hephaestus::CoupledCoefficient
{
public:
  static InputParameters validParams();

  MFEMVariableDependentFunctionCoefficient(const InputParameters & parameters);
  virtual ~MFEMVariableDependentFunctionCoefficient();

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

  double Eval(mfem::ElementTransformation & trans, const mfem::IntegrationPoint & ip) override;

  std::shared_ptr<mfem::Coefficient> getCoefficient() const override
  {
    return PlatypusUtils::dynamic_const_cast<mfem::Coefficient>(getSharedPtr());
  }

private:
  const Function & _func;
};
