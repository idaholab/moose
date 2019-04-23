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
  PorousFlowDictatorTest()
    : MooseObjectUnitTest("MooseUnitApp"),
      _linear_lagrange(FEType(Utility::string_to_enum<Order>("FIRST"),
                              Utility::string_to_enum<FEFamily>("LAGRANGE"))),
      _constant_monomial(FEType(Utility::string_to_enum<Order>("CONSTANT"),
                                Utility::string_to_enum<FEFamily>("MONOMIAL")))
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    _fe_problem->addVariable("var0", _linear_lagrange, 1.0);
    _fe_problem->addVariable("var1", _linear_lagrange, 1.0);
    _fe_problem->addVariable("var2", _linear_lagrange, 1.0);
    _fe_problem->addVariable("var3", _linear_lagrange, 1.0);
    _fe_problem->addVariable("var4", _linear_lagrange, 1.0);
    _fe_problem->addVariable("var5", _linear_lagrange, 1.0);
    _fe_problem->addVariable("var_different_fe_type", _constant_monomial, 1.0);
    _fe_problem->addAuxVariable("aux_var", _linear_lagrange);

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
    _dictator_no_fetype = &_fe_problem->getUserObject<PorousFlowDictator>("dictator_no_fetype");
  }

  const FEType _linear_lagrange;
  const FEType _constant_monomial;
  const PorousFlowDictator * _dictator;
  const PorousFlowDictator * _dictator_no_fetype;
};

