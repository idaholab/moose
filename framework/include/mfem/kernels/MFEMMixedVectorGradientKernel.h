#pragma once
#include "MFEMMixedBilinearFormKernel.h"

/*
 * \f[
 * (\sigma \nabla V, u')
 * \f]
 */
class MFEMMixedVectorGradientKernel : public MFEMMixedBilinearFormKernel
{
public:
  static InputParameters validParams();

  MFEMMixedVectorGradientKernel(const InputParameters & parameters);
  ~MFEMMixedVectorGradientKernel() override = default;

  virtual mfem::BilinearFormIntegrator * createIntegrator() override;

protected:
  std::string _coef_name;
  mfem::Coefficient & _coef;
};
