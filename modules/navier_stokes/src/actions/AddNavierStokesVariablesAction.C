//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddNavierStokesVariablesAction.h"

// MOOSE includes
#include "AddVariableAction.h"
#include "FEProblem.h"
#include "MooseMesh.h"

#include "libmesh/fe.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("NavierStokesApp",
                    AddNavierStokesVariablesAction,
                    "add_navier_stokes_variables");

template <>
InputParameters
validParams<AddNavierStokesVariablesAction>()
{
  InputParameters params = validParams<NSAction>();

  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies(), "LAGRANGE");
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders(), "FIRST");
  params.addClassDescription("This class allows us to have a section of the input file like the "
                             "following which automatically adds all the required nonlinear "
                             "variables with the appropriate scaling.");
  params.addParam<std::vector<SubdomainName>>(
      "block", "The list of block ids (SubdomainID) on which this action will be applied");
  params.addParam<MooseEnum>(
      "family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE shape function to use "
                             "for this variable (additional orders not listed are "
                             "allowed)");
  params.addRequiredParam<std::vector<Real>>(
      "scaling", "Specifies a scaling factor to apply to this variable");

  return params;
}

AddNavierStokesVariablesAction::AddNavierStokesVariablesAction(InputParameters parameters)
  : NSAction(parameters),
    _scaling(getParam<std::vector<Real>>("scaling")),
    _blocks(getParam<std::vector<SubdomainName>>("block"))
{
}

AddNavierStokesVariablesAction::~AddNavierStokesVariablesAction() {}

void
AddNavierStokesVariablesAction::act()
{
  // Call the base class's act() function to initialize the _vars and _auxs names.
  NSAction::act();

  // Make sure the number of scaling parameters matches the number of variables
  if (_scaling.size() != _vars.size())
    mooseError("Must provide a scaling parameter for each variable.");

  // All variables have the same type
  FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                 Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));

  std::set<SubdomainID> _block_ids;
  for (const auto & subdomain_name : _blocks)
  {
    SubdomainID id = _mesh->getSubdomainID(subdomain_name);
    _block_ids.insert(id);
  }

  auto var_type = AddVariableAction::determineType(fe_type, 1);
  auto base_params = _factory.getValidParams(var_type);
  base_params.set<MooseEnum>("order") = fe_type.order.get_order();
  base_params.set<MooseEnum>("family") = Moose::stringify(fe_type.family);
  if (_block_ids.size() != 0)
    for (const SubdomainID & id : _block_ids)
      base_params.set<std::vector<SubdomainName>>("block").push_back(Moose::stringify(id));

  // Add the variables to the FEProblemBase
  for (unsigned int i = 0; i < _vars.size(); ++i)
  {
    InputParameters var_params(base_params);
    var_params.set<std::vector<Real>>("scaling") = {_scaling[i]};
    _problem->addVariable(var_type, _vars[i], var_params);
  }

  // Add Aux variables.  These are all required in order for the code
  // to run, so they should not be independently selectable by the
  // user.
  for (const auto & aux_name : _auxs)
    _problem->addAuxVariable(var_type, aux_name, base_params);
}
