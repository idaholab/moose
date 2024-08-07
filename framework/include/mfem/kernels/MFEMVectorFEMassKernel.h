#pragma once
#include "MFEMBilinearFormKernel.h"
#include "kernels.h"

/*
(βu, u')
*/
class MFEMVectorFEMassKernel : public MFEMBilinearFormKernel
{
public:
  static InputParameters validParams();

  MFEMVectorFEMassKernel(const InputParameters & parameters);
  ~MFEMVectorFEMassKernel() override {}

  virtual mfem::BilinearFormIntegrator * createIntegrator() override;

protected:
  std::string _coef_name;
  mfem::Coefficient * _coef{nullptr};
};
