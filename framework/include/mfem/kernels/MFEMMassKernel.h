#pragma once
#include "MFEMKernel.h"

/*
(βu, u')
*/
class MFEMMassKernel : public MFEMKernel<mfem::BilinearFormIntegrator>
{
public:
  static InputParameters validParams();

  MFEMMassKernel(const InputParameters & parameters);
  ~MFEMMassKernel() override {}

  virtual mfem::BilinearFormIntegrator * createIntegrator() override;

protected:
  std::string _coef_name;
  mfem::Coefficient & _coef;
};
