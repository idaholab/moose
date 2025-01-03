#pragma once
#include "MFEMKernel.h"

/*
 * \f[
 * (\alpha \nabla \times u, \nabla \times u')
 * \f]
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
