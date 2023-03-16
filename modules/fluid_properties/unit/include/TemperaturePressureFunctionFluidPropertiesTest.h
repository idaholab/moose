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
#include "TemperaturePressureFunctionFluidProperties.h"
#include "MooseParsedFunction.h"

class TemperaturePressureFunctionFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  TemperaturePressureFunctionFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp")
  {
    InputParameters params = _factory.getValidParams("ParsedFunction");
    params.set<std::string>("_object_name") = "k_function";
    params.set<FEProblem *>("_fe_problem") = _fe_problem.get();
    params.set<FEProblemBase *>("_fe_problem_base") = _fe_problem.get();
    params.set<SubProblem *>("_subproblem") = _fe_problem.get();
    params.set<std::string>("expression") = "14 + 2e-2 * x + 3e-5 * y";
    params.set<std::string>("_type") = "MooseParsedFunction";
    _fe_problem->addFunction("ParsedFunction", "k_function", params);

    params.set<std::string>("expression") = "1400 + 2.5 * x + 32e-5 * y";
    params.set<std::string>("_object_name") = "rho_function";
    _fe_problem->addFunction("ParsedFunction", "rho_function", params);

    params.set<std::string>("expression") = "1e-3 + 1e-5 * x - 3e-9 * y";
    params.set<std::string>("_object_name") = "mu_function";
    _fe_problem->addFunction("ParsedFunction", "mu_function", params);

    _fe_problem->getFunction("k_function").initialSetup();
    _fe_problem->getFunction("rho_function").initialSetup();
    _fe_problem->getFunction("mu_function").initialSetup();

    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters uo_params =
        _factory.getValidParams("TemperaturePressureFunctionFluidProperties");
    // Set the three functions and the specific isobaric heat capacity
    uo_params.set<FunctionName>("k") = "k_function";
    uo_params.set<FunctionName>("rho") = "rho_function";
    uo_params.set<FunctionName>("mu") = "mu_function";
    uo_params.set<Real>("cv") = 4186;

    _fe_problem->addUserObject("TemperaturePressureFunctionFluidProperties", "fp", uo_params);
    _fp = &_fe_problem->getUserObject<TemperaturePressureFunctionFluidProperties>("fp");
  }

  TemperaturePressureFunctionFluidProperties * _fp;
};
