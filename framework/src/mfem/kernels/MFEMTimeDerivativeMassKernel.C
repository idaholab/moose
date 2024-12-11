#include "MFEMTimeDerivativeMassKernel.h"

registerMooseObject("PlatypusApp", MFEMTimeDerivativeMassKernel);

InputParameters
MFEMTimeDerivativeMassKernel::validParams()
{
  InputParameters params = MFEMMassKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k \\dot{u}, v)_\\Omega$ "
                             "arising from the weak form of the operator "
                             "$k \\dot{u}$.");
  return params;
}

MFEMTimeDerivativeMassKernel::MFEMTimeDerivativeMassKernel(const InputParameters & parameters)
  : MFEMMassKernel(parameters), _var_dot_name(platypus::GetTimeDerivativeName(_test_var_name))
{
}
