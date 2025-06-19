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
#include "NEML2Kernel.h"

InputParameters
NEML2Kernel::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addRequiredParam<UserObjectName>(
      "assembly", "The NEML2Assembly object to use to provide assembly information");
  params.addRequiredParam<UserObjectName>(
      "fe", "The NEML2FEInterpolation object to use to couple variables");

  return params;
}

NEML2Kernel::NEML2Kernel(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _neml2_assembly(getUserObject<NEML2Assembly>("assembly")),
    _fe(getUserObject<NEML2FEInterpolation>("fe"))
{
}

void
NEML2Kernel::execute()
{
  TIME_SECTION("execute", 1, "NEML2 kernel execution");

  forward();
}

#endif
