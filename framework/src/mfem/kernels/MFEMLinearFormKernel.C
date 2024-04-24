#include "MFEMLinearFormKernel.h"

registerMooseObject("PlatypusApp", MFEMLinearFormKernel);

InputParameters
MFEMLinearFormKernel::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.registerBase("Kernel");
  params.addParam<std::string>("variable", "Variable on which to apply the kernel");

  return params;
}

MFEMLinearFormKernel::MFEMLinearFormKernel(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
}

MFEMLinearFormKernel::~MFEMLinearFormKernel() {}
