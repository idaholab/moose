#ifdef MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

/*
 * \f[
 * (\sigma u, \nabla V')
 * \f]
 */
class MFEMVectorFEWeakDivergenceKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMVectorFEWeakDivergenceKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;

protected:
  const MFEMScalarCoefficientName & _coef_name;
  mfem::Coefficient & _coef;
};

#endif
