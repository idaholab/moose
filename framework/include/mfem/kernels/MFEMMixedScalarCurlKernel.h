#pragma once
#include "MFEMKernel.h"

/*
(λ∇×u, v)
*/
class MFEMMixedScalarCurlKernel : public MFEMKernel<mfem::BilinearFormIntegrator>
{
public:
  static InputParameters validParams();

  MFEMMixedScalarCurlKernel(const InputParameters & parameters);
  ~MFEMMixedScalarCurlKernel() override {}

  virtual mfem::BilinearFormIntegrator * createIntegrator() override;

protected:
  std::string _coef_name;
  mfem::Coefficient & _coef;
};
