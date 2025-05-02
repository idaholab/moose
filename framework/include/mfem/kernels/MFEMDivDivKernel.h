#ifdef MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

/*
 * \f[
 * (\alpha \nabla \cdot u, \nabla \cdot u')
 * \f]
 */
class MFEMDivDivKernel : public MFEMKernel
{
public:
  static InputParameters validParams();

  MFEMDivDivKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;

protected:
  const std::string _coef_name;
  mfem::Coefficient & _coef;
};

#endif
