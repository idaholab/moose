/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "AddElementalFieldAction.h"
#include "FEProblem.h"
#include "MooseMesh.h"

// libmesh includes
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
