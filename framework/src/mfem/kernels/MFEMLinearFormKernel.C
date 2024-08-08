#include "MFEMLinearFormKernel.h"

InputParameters
MFEMLinearFormKernel::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();

  params.registerBase("Kernel");
  params.addParam<std::string>("variable", "Variable on which to apply the kernel");

  return params;
}

MFEMLinearFormKernel::MFEMLinearFormKernel(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters)
{
}
