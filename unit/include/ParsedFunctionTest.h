//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectUnitTest.h"

#include "MooseParsedFunctionWrapper.h"

class MooseParsedFunction;

class ParsedFunctionTest : public MooseObjectUnitTest
{
public:
  ParsedFunctionTest() : MooseObjectUnitTest("MooseUnitApp") {}

protected:
  libMesh::ParsedFunction<Real> * fptr(MooseParsedFunction & f);
  InputParameters getParams();
  MooseParsedFunction & buildFunction(InputParameters & params);

  unsigned int function_index = 0;
};
