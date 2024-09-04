#include "MFEMMixedVectorGradientKernel.h"

registerMooseObject("PlatypusApp", MFEMMixedVectorGradientKernel);

InputParameters
MFEMMixedVectorGradientKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "The scaled gradient operator for the mixed form "
      "$(k\\nabla q, \\vec u')$ with $\\vec u$ a vector FE type, to be added to an MFEM problem");

  params.addParam<std::string>("coefficient", "Name of property k to use.");

  return params;
}

MFEMMixedVectorGradientKernel::MFEMMixedVectorGradientKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    // FIXME: The MFEM bilinear form can also handle vector and matrix
    // coefficients, so ideally we'd handle all three too.
    _coef(getMFEMProblem().getProperties().getScalarProperty(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMMixedVectorGradientKernel::createIntegrator()
{
  return new mfem::MixedVectorGradientIntegrator(_coef);
}
