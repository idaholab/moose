#ifdef MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

/*
 * \f[
 * (\sigma \nabla q, \nabla q')
 * \f]
 */
class MFEMDiffusionKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMDiffusionKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;

protected:
  const MFEMScalarCoefficientName & _coef_name;
  mfem::Coefficient & _coef;
};

#endif
