//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalVariablesAction.h"
#include "AddVariableAction.h"
#include "Conversion.h"
#include "Factory.h"
#include "FEProblem.h"
#include "NonlinearSystemBase.h"

#include "libmesh/string_to_enum.h"

registerMooseAction("PhaseFieldApp", PolycrystalVariablesAction, "add_variable");
registerMooseAction("PhaseFieldApp", PolycrystalVariablesAction, "copy_nodal_vars");
registerMooseAction("PhaseFieldApp", PolycrystalVariablesAction, "check_copy_nodal_vars");

InputParameters
PolycrystalVariablesAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set up order parameter variables for a polycrystal simulation");
  // Get MooseEnums for the possible order/family options for this variable
  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>("family",
                             families,
                             "Specifies the family of FE "
                             "shape function to use for the order parameters");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE "
                             "shape function to use for the order parameters");
  params.addParam<bool>(
      "initial_from_file",
      false,
      "Take the initial condition of all polycrystal variables from the mesh file");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to this variable");
  params.addRequiredParam<unsigned int>("op_num",
                                        "specifies the number of order parameters to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "Block restriction for the variables and kernels");
  return params;
}

PolycrystalVariablesAction::PolycrystalVariablesAction(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{
}

void
PolycrystalVariablesAction::act()
{
  // take initial values from file?
  bool initial_from_file = getParam<bool>("initial_from_file");

  // Loop through the number of order parameters
  for (unsigned int op = 0; op < _op_num; op++)
  {
    // Create variable names
    std::string var_name = _var_name_base + Moose::stringify(op);

    // Add the variable
    if (_current_task == "add_variable")
    {
      auto fe_type = AddVariableAction::feType(_pars);
      auto type = AddVariableAction::variableType(fe_type);
      auto var_params = _factory.getValidParams(type);

      var_params.applySpecificParameters(_pars, {"order", "family", "block"});
      var_params.set<std::vector<Real>>("scaling") = {_pars.get<Real>("scaling")};
      _problem->addVariable(type, var_name, var_params);
    }

    // Setup initial from file if requested
    if (initial_from_file)
    {
      if (_current_task == "check_copy_nodal_vars")
        _app.setExodusFileRestart(true);

      if (_current_task == "copy_nodal_vars")
      {
        auto * system = &_problem->getNonlinearSystemBase();
        system->addVariableToCopy(var_name, var_name, "LATEST");
      }
    }
  }
}
