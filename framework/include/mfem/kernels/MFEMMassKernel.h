#ifdef MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

/*
 * \f[
 * (\beta u, u')
 * \f]
 */
class MFEMMassKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMMassKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;

protected:
  const MFEMScalarCoefficientName & _coef_name;
  mfem::Coefficient & _coef;
};

#endif
