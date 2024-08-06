#include "MFEMVectorFEDomainLFKernel.h"

registerMooseObject("PlatypusApp", MFEMVectorFEDomainLFKernel);

InputParameters
MFEMVectorFEDomainLFKernel::validParams()
{
  InputParameters params = MFEMLinearFormKernel::validParams();
  params.addClassDescription("A volumetric function ($f$), with the weak "
                             "form of $ (f, u_h), to be added to an MFEM problem");

  params.addParam<std::string>("vector_coefficient", "Name of MFEM vector coefficient f.");

  return params;
}

MFEMVectorFEDomainLFKernel::MFEMVectorFEDomainLFKernel(const InputParameters & parameters)
  : MFEMLinearFormKernel(parameters),
    _kernel_params{{{"VariableName", getParam<std::string>("variable")},
                    {"VectorCoefficientName", getParam<std::string>("vector_coefficient")}}},
    _kernel{std::make_shared<platypus::VectorFEDomainLFKernel>(_kernel_params)}
{
}
