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
#include "NEML2PreKernel.h"

InputParameters
NEML2PreKernel::validParams()
{
  InputParameters params = NEML2Kernel::validParams();
  params += MOOSEToNEML2::validParams();

  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum = {EXEC_INITIAL, EXEC_LINEAR};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

NEML2PreKernel::NEML2PreKernel(const InputParameters & parameters)
  : NEML2Kernel(parameters), MOOSEToNEML2(parameters)
{
}

#endif
