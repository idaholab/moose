//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FormLossFromFunction1Phase.h"

registerMooseObject("ThermalHydraulicsApp", FormLossFromFunction1Phase);

InputParameters
FormLossFromFunction1Phase::validParams()
{
  InputParameters params = FormLoss1PhaseBase::validParams();
  params.addRequiredParam<FunctionName>("K_prime",
                                        "Form loss coefficient per unit length function [1/m]");

  params.addClassDescription(
      "Prescribe a form loss over a 1-phase flow channel given by a function");

  return params;
}

FormLossFromFunction1Phase::FormLossFromFunction1Phase(const InputParameters & params)
  : FormLoss1PhaseBase(params)
{
}

void
FormLossFromFunction1Phase::addMooseObjects()
{
  FormLoss1PhaseBase::addMooseObjects();

  {
    const std::string class_name = "ADGenericFunctionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<std::vector<std::string>>("prop_names") = {"K_prime"};
    params.set<std::vector<FunctionName>>("prop_values") = {getParam<FunctionName>("K_prime")};
    getTHMProblem().addMaterial(class_name, genName(name(), "k_prime_material"), params);
  }
}
