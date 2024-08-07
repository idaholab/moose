#include "MFEMCurlCurlKernel.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMCurlCurlKernel);

InputParameters
MFEMCurlCurlKernel::validParams()
{
  InputParameters params = MFEMBilinearFormKernel::validParams();
  params.addClassDescription(
      "The curl curl operator ($-k\\nabla \\times \\nabla \\times u$), with the weak "
      "form of $ (k\\nabla \\times \\phi_i, \\nabla \\times u_h), to be added to an MFEM problem");

  params.addParam<std::string>("coefficient",
                               "Name of MFEM coefficient k to multiply the Laplacian by");

  return params;
}

MFEMCurlCurlKernel::MFEMCurlCurlKernel(const InputParameters & parameters)
  : MFEMBilinearFormKernel(parameters),
    _coef_name(getParam<std::string>("coefficient")),
    _coef(getMFEMProblem()._coefficients._scalars.Get(_coef_name))
{
}

mfem::BilinearFormIntegrator *
MFEMCurlCurlKernel::createIntegrator()
{
  return new mfem::CurlCurlIntegrator(*_coef);
}
