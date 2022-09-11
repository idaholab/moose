//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetupResidualDebugAction.h"

// MOOSE includes
#include "ActionWarehouse.h"
#include "Factory.h"
#include "FEProblem.h"
#include "MooseVariableFE.h"
#include "NonlinearSystemBase.h"
#include "Conversion.h"
#include "AddVariableAction.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("MooseApp", SetupResidualDebugAction, "setup_residual_debug");

InputParameters
SetupResidualDebugAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<std::vector<NonlinearVariableName>>(
      "show_var_residual", "Variables for which residuals will be sent to the output file.");
  params.addClassDescription(
      "Adds the necessary objects for computing the residuals for individual variables.");
  return params;
}

SetupResidualDebugAction::SetupResidualDebugAction(const InputParameters & parameters)
  : Action(parameters),
    _show_var_residual(getParam<std::vector<NonlinearVariableName>>("show_var_residual"))
{
}

void
SetupResidualDebugAction::act()
{
  if (_problem.get() == NULL)
    return;

  if (!_show_var_residual.empty())
    _problem->getNonlinearSystemBase().debuggingResiduals(true);

  // debug variable residuals
  for (const auto & var_name : _show_var_residual)
  {
    // add aux-variable
    auto order = AddVariableAction::getNonlinearVariableOrders();
    auto family = AddVariableAction::getNonlinearVariableFamilies();

    MooseVariableFEBase & var = _problem->getVariable(
        0, var_name, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD);
    auto fe_type = var.feType();
    order = Utility::enum_to_string<Order>(fe_type.order);
    family = Utility::enum_to_string(fe_type.family);

    InputParameters params = _factory.getValidParams("DebugResidualAux");
    const std::set<SubdomainID> & subdomains = var.activeSubdomains();

    std::stringstream aux_var_ss;
    aux_var_ss << "residual_" << var.name();
    std::string aux_var_name = aux_var_ss.str();

    auto var_params = _factory.getValidParams("MooseVariable");
    var_params.set<MooseEnum>("family") = family;
    var_params.set<MooseEnum>("order") = order;

    if (subdomains.empty())
      _problem->addAuxVariable("MooseVariable", aux_var_name, var_params);
    else
    {
      std::vector<SubdomainName> block_names;
      block_names.reserve(subdomains.size());
      for (const SubdomainID & id : subdomains)
        block_names.push_back(Moose::stringify(id));
      params.set<std::vector<SubdomainName>>("block") = block_names;
      var_params.set<std::vector<SubdomainName>>("block") = block_names;

      _problem->addAuxVariable("MooseVariable", aux_var_name, var_params);
    }

    // add aux-kernel
    std::stringstream kern_ss;
    kern_ss << "residual_" << var.name() << "_kernel";
    std::string kern_name = kern_ss.str();

    params.set<AuxVariableName>("variable") = aux_var_name;
    params.set<NonlinearVariableName>("debug_variable") = var.name();
    params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_TIMESTEP_END};
    _problem->addAuxKernel("DebugResidualAux", kern_name, params);
  }
}
