//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FormLossFromExternalApp1Phase.h"

registerMooseObject("ThermalHydraulicsApp", FormLossFromExternalApp1Phase);

InputParameters
FormLossFromExternalApp1Phase::validParams()
{
  InputParameters params = FormLoss1PhaseBase::validParams();

  params.addClassDescription("Apply a distributed form loss over a 1-phase flow channel computed "
                             "by an external application.");

  return params;
}

FormLossFromExternalApp1Phase::FormLossFromExternalApp1Phase(const InputParameters & params)
  : FormLoss1PhaseBase(params), _K_prime_var_name("K_prime")
{
}

void
FormLossFromExternalApp1Phase::addVariables()
{
  getTHMProblem().addSimVariable(
      false, _K_prime_var_name, libMesh::FEType(FIRST, LAGRANGE), _flow_channel_subdomains);
}

void
FormLossFromExternalApp1Phase::addMooseObjects()
{
  FormLoss1PhaseBase::addMooseObjects();

  {
    const std::string class_name = "ADCoupledVariableValueMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<MaterialPropertyName>("prop_name") = "K_prime";
    params.set<std::vector<VariableName>>("coupled_variable") = {_K_prime_var_name};
    getTHMProblem().addMaterial(class_name, genName(name(), "k_prime_material"), params);
  }
}
