#ifdef MFEM_ENABLED

#include "MFEMMixedBilinearFormKernel.h"

InputParameters
MFEMMixedBilinearFormKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Base class for mixed bilinear form kernels, allowing different trial and test variables.");
  params.addParam<VariableName>(
      "trial_variable",
      "The trial variable this kernel is acting on and which will be solved for. If empty "
      "(default), it will be the same as the test variable.");
  params.addParam<bool>(
      "transpose", false, "If true, adds the transpose of the integrator to the system instead.");
  return params;
}

MFEMMixedBilinearFormKernel::MFEMMixedBilinearFormKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _trial_var_name(isParamValid("trial_variable") ? getParam<VariableName>("trial_variable")
                                                   : _test_var_name),
    _transpose(getParam<bool>("transpose"))
{
}

const VariableName &
MFEMMixedBilinearFormKernel::getTrialVariableName() const
{
  return _trial_var_name;
}

mfem::BilinearFormIntegrator *
MFEMMixedBilinearFormKernel::createBFIntegrator()
{
  return _transpose ? new mfem::TransposeIntegrator(createMBFIntegrator()) : createMBFIntegrator();
}
#endif
