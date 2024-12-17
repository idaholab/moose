#ifdef MFEM_ENABLED

#include "MFEMMixedVectorGradientKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMMixedVectorGradientKernel);

InputParameters
MFEMMixedVectorGradientKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(k\\vec\\nabla u, \\vec v)_\\Omega$ "
      "arising from the weak form of the gradient operator "
      "$k\\vec \\nabla u$.");
  params.addParam<platypus::MFEMScalarCoefficientName>("coefficient", "Name of property k to use.");
  return params;
}

MFEMMixedVectorGradientKernel::MFEMMixedVectorGradientKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<platypus::MFEMScalarCoefficientName>("coefficient")),
    // FIXME: The MFEM bilinear form can also handle vector and matrix
    // coefficients, so ideally we'd handle all three too.
    _coef(getScalarProperty(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMMixedVectorGradientKernel::createIntegrator()
{
  return new mfem::MixedVectorGradientIntegrator(_coef);
}

#endif
