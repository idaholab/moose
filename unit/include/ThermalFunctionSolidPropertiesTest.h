//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONALTHERMALSOLIDPROPERTIESTEST_H
#define FUNCTIONALTHERMALSOLIDPROPERTIESTEST_H

#include "MooseObjectUnitTest.h"
#include "ThermalFunctionSolidProperties.h"
#include "MooseParsedFunction.h"

class ThermalFunctionSolidPropertiesTest : public MooseObjectUnitTest
{
public:
  ThermalFunctionSolidPropertiesTest() : MooseObjectUnitTest("MooseUnitApp")
  {
    registerObjects(_factory);
    buildObjects();
  }

protected:
  void registerObjects(Factory & factory) { registerUserObject(ThermalFunctionSolidProperties); }

  void buildObjects()
  {
    InputParameters k_pars = _factory.getValidParams("ParsedFunction");
    k_pars.set<std::string>("value") = "(t+500.0)/1000.0";
    _fe_problem->addFunction("ParsedFunction", "k", k_pars);
    Function & k = _fe_problem->getFunction("k");
    k.initialSetup();

    InputParameters cp_pars = _factory.getValidParams("ParsedFunction");
    cp_pars.set<std::string>("value") = "1000.0";
    _fe_problem->addFunction("ParsedFunction", "cp", cp_pars);
    Function & cp = _fe_problem->getFunction("cp");
    cp.initialSetup();

    InputParameters rho_pars = _factory.getValidParams("ParsedFunction");
    rho_pars.set<std::string>("value") = "20.0*exp(t/1000.0)";
    _fe_problem->addFunction("ParsedFunction", "rho", rho_pars);
    Function & rho = _fe_problem->getFunction("rho");
    rho.initialSetup();

    InputParameters uo_pars = _factory.getValidParams("ThermalFunctionSolidProperties");
    uo_pars.set<FunctionName>("k") = "k";
    uo_pars.set<FunctionName>("cp") = "cp";
    uo_pars.set<FunctionName>("rho") = "rho";
    _fe_problem->addUserObject("ThermalFunctionSolidProperties", "sp", uo_pars);
    _sp = &_fe_problem->getUserObject<ThermalFunctionSolidProperties>("sp");
  }

  const ThermalFunctionSolidProperties * _sp;
};

#endif /* FUNCTIONALTHERMALSOLIDPROPERTIESTEST_H */
