//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

// MOOSE includes
#include "TorchPostKernel.h"

InputParameters
TorchPostKernel::validParams()
{
  InputParameters params = TorchKernel::validParams();
  params.addRequiredParam<UserObjectName>(
      "executor",
      "The NEML2ModelExecutor used to perform the constitutive update (where stress is an output "
      "variable). If you are using the NEML2 action, the parameter executor_name can be used to "
      "specify the name of the NEML2ModelExecutor.");

  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_KERNELS);
  exec_enum = {EXEC_INITIAL, EXEC_PRE_KERNELS};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

TorchPostKernel::TorchPostKernel(const InputParameters & parameters)
  : TorchKernel(parameters), _constitutive(getUserObject<NEML2ModelExecutor>("executor"))
{
}

#endif // NEML2_ENABLED
