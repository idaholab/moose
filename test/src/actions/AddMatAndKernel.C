//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMatAndKernel.h"
#include "FEProblem.h"
#include "Factory.h"
#include "AddVariableAction.h"

template <>
InputParameters
validParams<AddMatAndKernel>()
{
  InputParameters params = validParams<AddVariableAction>();
  params.addParam<bool>("verbose",
                        false,
                        "Boolean which causes the Action to print information"
                        "about what its doing");
  return params;
}

AddMatAndKernel::AddMatAndKernel(const InputParameters & params)
  : AddVariableAction(params), _verbose(getParam<bool>("verbose"))
{
}

void
AddMatAndKernel::act()
{
  const std::string var_name = "var1";
  if (_current_task == "add_variable")
  {
    if (_verbose)
      _console << "AddMatAndKernel: Adding variable " << var_name << '\n';

    addVariable(var_name);
  }
  else if (_current_task == "add_kernel")
  {
    const std::string kernel_name = "kern1";

    if (_verbose)
      _console << "AddMatAndKernel: Adding kernel " << kernel_name << '\n';

    InputParameters params = _factory.getValidParams("MatDiffusion");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<MaterialPropertyName>("prop_name") = "prop1";
    _problem->addKernel("MatDiffusion", kernel_name, params);
  }
  else if (_current_task == "add_material")
  {
    const std::string mat_name = "mat1";
    if (_verbose)
      _console << "AddMatAndKernel: Adding material " << mat_name << '\n';

    InputParameters params = _factory.getValidParams("GenericConstantMaterial");
    params.set<std::vector<std::string>>("prop_names") = {"prop1"};
    params.set<std::vector<Real>>("prop_values") = {42};
    _problem->addMaterial("GenericConstantMaterial", mat_name, params);
  }
}
