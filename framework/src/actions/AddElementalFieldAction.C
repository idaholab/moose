//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AddElementalFieldAction.h"
#include "FEProblem.h"
#include "MooseMesh.h"

#include "libmesh/fe.h"

template <>
InputParameters
validParams<AddElementalFieldAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::vector<SubdomainName>>("block", "The block id where this object lives.");

  return params;
}

AddElementalFieldAction::AddElementalFieldAction(InputParameters params) : Action(params) {}

void
AddElementalFieldAction::act()
{
  std::set<SubdomainID> blocks;
  std::vector<SubdomainName> block_param = getParam<std::vector<SubdomainName>>("block");
  for (const auto & subdomain_name : block_param)
  {
    SubdomainID blk_id = _problem->mesh().getSubdomainID(subdomain_name);
    blocks.insert(blk_id);
  }

  FEType fe_type(CONSTANT, MONOMIAL);

  std::string variable = name();

  if (blocks.empty())
    _problem->addAuxVariable(variable, fe_type);
  else
    _problem->addAuxVariable(variable, fe_type, &blocks);
}
