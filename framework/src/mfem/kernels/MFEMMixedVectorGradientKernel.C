#include "MFEMMixedVectorGradientKernel.h"

registerMooseObject("PlatypusApp", MFEMMixedVectorGradientKernel);

InputParameters
MFEMMixedVectorGradientKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "The scaled gradient operator for the mixed form "
      "$(k\\nabla q, \\vec u')$ with $\\vec u$ a vector FE type, to be added to an MFEM problem");

  params.addParam<std::string>("coefficient", "Name of MFEM coefficient k to use.");

  return params;
}

MFEMMixedVectorGradientKernel::MFEMMixedVectorGradientKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    _coef(getMFEMProblem()._coefficients._scalars.Get(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMMixedVectorGradientKernel::createIntegrator()
{
  return new mfem::MixedVectorGradientIntegrator(*_coef);
}
