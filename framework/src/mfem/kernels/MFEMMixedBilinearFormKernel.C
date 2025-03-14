#ifdef MFEM_ENABLED

#include "MFEMMixedBilinearFormKernel.h"

InputParameters
MFEMMixedBilinearFormKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Base class for mixed bilinear form kernels, allowing different trial and test variables.");
  params.addParam<std::string>(
      "trial_variable",
      "",
      "The trial variable this kernel is acting on and which will be solved for. If empty "
      "(default), it will be the same as the test variable.");
  return params;
}

MFEMMixedBilinearFormKernel::MFEMMixedBilinearFormKernel(const InputParameters & parameters)
  : MFEMKernel(parameters), _trial_var_name(getParam<std::string>("trial_variable"))
{
  if (_trial_var_name == "")
  {
    _trial_var_name = _test_var_name;
  }
}

const std::string &
MFEMMixedBilinearFormKernel::getTrialVariableName() const
{
  return _trial_var_name;
}

#endif
