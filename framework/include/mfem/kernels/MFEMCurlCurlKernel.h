#pragma once
#include "MFEMKernel.h"

/*
(α∇×u, ∇×u')
*/
class MFEMCurlCurlKernel : public MFEMKernel<mfem::BilinearFormIntegrator>
{
public:
  static InputParameters validParams();

  MFEMCurlCurlKernel(const InputParameters & parameters);
  ~MFEMCurlCurlKernel() override {}

  virtual mfem::BilinearFormIntegrator * createIntegrator() override;

protected:
  std::string _coef_name;
  mfem::Coefficient & _coef;
};
