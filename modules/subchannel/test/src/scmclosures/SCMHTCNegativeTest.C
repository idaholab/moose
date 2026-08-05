//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMHTCNegativeTest.h"

registerMooseObject("SubChannelTestApp", SCMHTCNegativeTest);

InputParameters
SCMHTCNegativeTest::validParams()
{
  InputParameters params = SCMHTCClosureBase::validParams();
  params.addClassDescription("Test closure that returns a negative Nusselt number.");
  return params;
}

SCMHTCNegativeTest::SCMHTCNegativeTest(const InputParameters & parameters)
  : SCMHTCClosureBase(parameters)
{
}

Real
SCMHTCNegativeTest::computeNusseltNumber(const FrictionStruct &, const NusseltStruct &) const
{
  return -1.0;
}
