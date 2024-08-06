#include "MFEMCurlCurlKernel.h"

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
    _kernel_params{{{"VariableName", getParam<std::string>("variable")},
                    {"CoefficientName", getParam<std::string>("coefficient")}}},
    _kernel{std::make_shared<platypus::CurlCurlKernel>(_kernel_params)}
{
}
