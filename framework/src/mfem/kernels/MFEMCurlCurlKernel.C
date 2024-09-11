#include "MFEMCurlCurlKernel.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMCurlCurlKernel);

InputParameters
MFEMCurlCurlKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "The curl curl operator ($-k\\nabla \\times \\nabla \\times u$), with the weak "
      "form of $ (k\\nabla \\times \\phi_i, \\nabla \\times u_h), to be added to an MFEM problem");

  params.addParam<std::string>("coefficient", "Name of property k to multiply the Laplacian by");

  return params;
}

MFEMCurlCurlKernel::MFEMCurlCurlKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    // FIXME: The MFEM bilinear form can also handle vector and matrix
    // coefficients, so ideally we'd handle all three too.
    _coef(getMFEMProblem().getProperties().getScalarProperty(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMCurlCurlKernel::createIntegrator()
{
  return new mfem::CurlCurlIntegrator(_coef);
}
