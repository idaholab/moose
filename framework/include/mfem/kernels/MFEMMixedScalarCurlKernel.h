#ifdef MFEM_ENABLED

#pragma once
#include "MFEMMixedBilinearFormKernel.h"

/*
 * \f[
 * (\lambda \nabla \times u, v)
 * \f]
 */
class MFEMMixedScalarCurlKernel : public MFEMMixedBilinearFormKernel
{
public:
  static InputParameters validParams();

  MFEMMixedScalarCurlKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createIntegrator() override;

protected:
  const std::string _coef_name;
  mfem::Coefficient & _coef;
};

#endif
