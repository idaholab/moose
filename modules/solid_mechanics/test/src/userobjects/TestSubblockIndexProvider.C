//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestSubblockIndexProvider.h"

registerMooseObject("TensorMechanicsTestApp", TestSubblockIndexProvider);

InputParameters
TestSubblockIndexProvider::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  return params;
}

TestSubblockIndexProvider::TestSubblockIndexProvider(const InputParameters & params)
  : GeneralUserObject(params)
{
}

unsigned int
TestSubblockIndexProvider::getSubblockIndex(const Elem & elem) const
{
  Point p = *elem.node_ptr(0);

  if (MooseUtils::relativeFuzzyLessThan(p(0), 0.5))
    return 0;
  else
    return 1;
}

unsigned int
TestSubblockIndexProvider::getMaxSubblockIndex() const
{
  return 1;
}
