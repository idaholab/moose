#include "MFEMVectorFEMassKernel.h"

registerMooseObject("PlatypusApp", MFEMVectorFEMassKernel);

InputParameters
MFEMVectorFEMassKernel::validParams()
{
  InputParameters params = MFEMBilinearFormKernel::validParams();
  params.addClassDescription("The mass operator ($k u$), with the weak "
                             "form of $ (k \\phi_i, \\times u_h), to be added to an MFEM problem");

  params.addParam<std::string>("coefficient",
                               "Name of MFEM coefficient k to multiply the Laplacian by");

  return params;
}

MFEMVectorFEMassKernel::MFEMVectorFEMassKernel(const InputParameters & parameters)
  : MFEMBilinearFormKernel(parameters),
    _kernel_params{{{"VariableName", getParam<std::string>("variable")},
                    {"CoefficientName", getParam<std::string>("coefficient")}}},
    _kernel{std::make_shared<platypus::VectorFEMassKernel>(_kernel_params)}
{
}
