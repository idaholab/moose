//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

// MOOSE includes
#include "TorchKernel.h"

InputParameters
TorchKernel::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addRequiredParam<UserObjectName>(
      "assembly", "The TorchAssembly object to use to provide assembly information");
  params.addRequiredParam<UserObjectName>(
      "fe", "The TorchFEInterpolation object to use to couple variables");

  return params;
}

TorchKernel::TorchKernel(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _assembly(_fe_problem.getUserObject<TorchAssembly>("assembly", /*tid=*/0)),
    _fe(_fe_problem.getUserObject<TorchFEInterpolation>("fe", /*tid=*/0))
{
}

void
TorchKernel::execute()
{
  TIME_SECTION("execute", 1, "Torch FEM kernel execution");

  forward();
}

#endif // MOOSE_LIBTORCH_ENABLED
