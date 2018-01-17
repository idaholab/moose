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

#include "AddRelationshipManager.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<AddRelationshipManager>()
{
  return validParams<Action>();
}

AddRelationshipManager::AddRelationshipManager(InputParameters params) : Action(params) {}

void
AddRelationshipManager::act()
{
  Moose::RelationshipManagerType rm_type = _current_task == "add_geometric_rm"
                                               ? Moose::RelationshipManagerType::Geometric
                                               : Moose::RelationshipManagerType::Algebraic;

  const auto & all_action_ptrs = _awh.allActionBlocks();
  for (const auto & action_ptr : all_action_ptrs)
    action_ptr->addRelationshipManagers(rm_type);

  _app.attachRelationshipManagers(rm_type);
}
