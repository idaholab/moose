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
#include "Function.h"

class CircularAreaHydraulicDiameterFunctionTest : public MooseObjectUnitTest
{
public:
  CircularAreaHydraulicDiameterFunctionTest()
    : MooseObjectUnitTest("ThermalHydraulicsApp"), _Dh_name("Dh_fn")
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    // area
    const std::string A_name = "A_fn";
    {
      const std::string class_name = "ParsedFunction";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::string>("value") = "1 + x";
      _fe_problem->addFunction(class_name, A_name, params);
    }

    Function & A_fn = _fe_problem->getFunction(A_name);
    A_fn.initialSetup();

    // hydraulic diameter
    {
      const std::string class_name = "CircularAreaHydraulicDiameterFunction";
      const std::string Dh_name = "Dh_fn";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<FunctionName>("area_function") = A_name;
      _fe_problem->addFunction(class_name, Dh_name, params);
    }
  }

  const FunctionName _Dh_name;
};
