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

#include "AddIndicatorVariableAction.h"
#include "FEProblem.h"

// libmesh includes
#include "fe.h"

template<>
InputParameters validParams<AddIndicatorVariableAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::vector<SubdomainName> >("block", "The block id where this variable lives");
  params.addRequiredParam<std::string>("variable", "The name of the indicator field");
  return params;
}

AddIndicatorVariableAction::AddIndicatorVariableAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddIndicatorVariableAction::act()
{
  std::set<SubdomainID> blocks;
  std::vector<SubdomainName> block_param = getParam<std::vector<SubdomainName> >("block");
  for (std::vector<SubdomainName>::iterator it = block_param.begin(); it != block_param.end(); ++it)
  {
    SubdomainID blk_id = _problem->mesh().getSubdomainID(*it);
    blocks.insert(blk_id);
  }

  FEType fe_type(CONSTANT, MONOMIAL);

  std::string variable = getParam<std::string>("variable");

  if (blocks.empty())
    _problem->addAuxVariable(variable, fe_type);
  else
    _problem->addAuxVariable(variable, fe_type, &blocks);
}
