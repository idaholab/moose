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
#include "PorousFlowDictator.h"
#include "libmesh/string_to_enum.h"

class PorousFlowDictator;

class PorousFlowDictatorTest : public MooseObjectUnitTest
{
public:
  PorousFlowDictatorTest() : MooseObjectUnitTest("PorousFlowApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    auto var_params_ll = _factory.getValidParams("MooseVariable");
    var_params_ll.set<MooseEnum>("family") = "LAGRANGE";
    var_params_ll.set<MooseEnum>("order") = "FIRST";

    auto var_params_cm = _factory.getValidParams("MooseVariableConstMonomial");
    var_params_cm.set<MooseEnum>("family") = "MONOMIAL";
    var_params_cm.set<MooseEnum>("order") = "CONSTANT";

    _fe_problem->addVariable("MooseVariable", "var0", var_params_ll);
    _fe_problem->addVariable("MooseVariable", "var1", var_params_ll);
    _fe_problem->addVariable("MooseVariable", "var2", var_params_ll);
    _fe_problem->addVariable("MooseVariable", "var3", var_params_ll);
    _fe_problem->addVariable("MooseVariable", "var4", var_params_ll);
    _fe_problem->addVariable("MooseVariable", "var5", var_params_ll);
    _fe_problem->addVariable("MooseVariableConstMonomial", "var_different_fe_type", var_params_cm);
    _fe_problem->addAuxVariable("MooseVariable", "aux_var", var_params_ll);

    InputParameters params = _factory.getValidParams("PorousFlowDictator");
    params.set<std::vector<VariableName>>("porous_flow_vars") =
        std::vector<VariableName>{"var1", "var4", "var3"};
    params.set<unsigned>("number_fluid_phases") = 2;
    params.set<unsigned>("number_fluid_components") = 4;
    params.set<unsigned int>("number_aqueous_equilibrium") = 5;
    params.set<unsigned int>("number_aqueous_kinetic") = 6;
    params.set<unsigned int>("aqueous_phase_number") = 1;
    _fe_problem->addUserObject("PorousFlowDictator", "dictator_name", params);
    _dictator = &_fe_problem->getUserObject<PorousFlowDictator>("dictator_name");

    InputParameters params_no_fetype = _factory.getValidParams("PorousFlowDictator");
    params_no_fetype.set<std::vector<VariableName>>("porous_flow_vars") =
        std::vector<VariableName>{"var1", "var_different_fe_type"};
    params_no_fetype.set<unsigned>("number_fluid_phases") = 1;
    params_no_fetype.set<unsigned>("number_fluid_components") = 3;
    _fe_problem->addUserObject("PorousFlowDictator", "dictator_no_fetype", params_no_fetype);
    _dictator_no_fetype =
        &_fe_problem->getUserObject<PorousFlowDictator>("dictator_no_fetype");
  }

  const PorousFlowDictator * _dictator;
  const PorousFlowDictator * _dictator_no_fetype;
};
