/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AddNavierStokesVariablesAction.h"

// MOOSE includes
#include "AddVariableAction.h"
#include "FEProblem.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/fe.h"
#include "libmesh/string_to_enum.h"

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

  if (_block_ids.size() == 0)
  {
    // Add the variables to the FEProblemBase
    for (unsigned int i = 0; i < _vars.size(); ++i)
      _problem->addVariable(_vars[i], fe_type, _scaling[i]);

    // Add Aux variables.  These are all required in order for the code
    // to run, so they should not be independently selectable by the
    // user.
    for (const auto & aux_name : _auxs)
      _problem->addAuxVariable(aux_name, fe_type);
  }
  else
  {
    // Add the variables to the FEProblemBase
    for (unsigned int i = 0; i < _vars.size(); ++i)
      _problem->addVariable(_vars[i], fe_type, _scaling[i], &_block_ids);

    // Add Aux variables.  These are all required in order for the code
    // to run, so they should not be independently selectable by the
    // user.
    for (const auto & aux_name : _auxs)
      _problem->addAuxVariable(aux_name, fe_type, &_block_ids);
  }
}
