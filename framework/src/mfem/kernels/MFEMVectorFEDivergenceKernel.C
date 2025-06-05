#ifdef MFEM_ENABLED

#include "MFEMVectorFEDivergenceKernel.h"

registerMooseObject("MooseApp", MFEMVectorFEDivergenceKernel);

InputParameters
MFEMVectorFEDivergenceKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(k \\vec \\nabla \\cdot \\vec u, v)_\\Omega$ arising from the weak form "
      "of the divergence operator $k \\vec \\nabla \\cdot \\vec u$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "Name of property k to use.");
  return params;
}

MFEMVectorFEDivergenceKernel::MFEMVectorFEDivergenceKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters),
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient")))
{
}

mfem::BilinearFormIntegrator *
MFEMVectorFEDivergenceKernel::createMBFIntegrator()
{
  return new mfem::VectorFEDivergenceIntegrator(_coef);
}

#endif
