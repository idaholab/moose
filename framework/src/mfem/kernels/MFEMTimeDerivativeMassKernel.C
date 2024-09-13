#include "MFEMTimeDerivativeMassKernel.h"

registerMooseObject("PlatypusApp", MFEMTimeDerivativeMassKernel);

InputParameters
MFEMTimeDerivativeMassKernel::validParams()
{
  InputParameters params = MFEMMassKernel::validParams();
  return params;
}

MFEMTimeDerivativeMassKernel::MFEMTimeDerivativeMassKernel(const InputParameters & parameters)
  : MFEMMassKernel(parameters), _var_dot_name(platypus::GetTimeDerivativeName(_test_var_name))
{
}
