//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddUELVariables.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "AddVariableAction.h"
#include "AbaqusUELMesh.h"

#include "libmesh/fe_type.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("SolidMechanicsApp", AddUELVariables, "add_variable");

InputParameters
AddUELVariables::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Add variables from an Abaqus input");
  return params;
}

AddUELVariables::AddUELVariables(const InputParameters & params) : Action(params) {}

void
AddUELVariables::act()
{
  const auto fe_type = FEType{Utility::string_to_enum<Order>("FIRST"),
                              Utility::string_to_enum<FEFamily>("LAGRANGE")};
  auto type = AddVariableAction::variableType(fe_type);
  auto var_params = _factory.getValidParams(type);

  // get all variables
  const auto uel_mesh = std::dynamic_pointer_cast<AbaqusUELMesh>(_mesh);
  if (!uel_mesh)
    mooseError("Must use an AbaqusUELMesh for UEL support.");
  const auto uels = uel_mesh->getUELs();
  std::set<std::size_t> all_vars;
  for (const auto & uel : uels)
    for (const auto & node : uel.vars)
      for (const auto & var : node)
        all_vars.insert(var);

  // get all variable blocks
  const auto all_blocks = uel_mesh->getVarBlocks();

  // create all variables
  for (const auto & var : all_vars)
  {
    std::vector<SubdomainName> block;
    for (const auto b : all_blocks)
      if (b & (1 << var))
        block.push_back(Moose::stringify(b));

    var_params.set<std::vector<SubdomainName>>("block") = block;
    const auto var_name = "var_" + Moose::stringify(var + 1);
    _problem->addVariable(type, var_name, var_params);
  }
}
