//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMAddRelationshipManagersAction.h"
#include "RelationshipManager.h"

registerMooseAction("ThermalHydraulicsApp",
                    THMAddRelationshipManagersAction,
                    "THM:add_relationship_managers");

InputParameters
THMAddRelationshipManagersAction::validParams()
{
  InputParameters params = Action::validParams();
  params += THMAppInterface::validParams();

  params.addClassDescription("Adds relationship managers for THM.");

  return params;
}

THMAddRelationshipManagersAction::THMAddRelationshipManagersAction(const InputParameters & params)
  : Action(params), THMAppInterface(params)
{
}

void
THMAddRelationshipManagersAction::act()
{
  auto & thm_app = getTHMApp();

  const std::string class_name = "AugmentSparsityBetweenElements";
  auto params = _factory.getValidParams(class_name);
  params.set<Moose::RelationshipManagerType>("rm_type") =
      Moose::RelationshipManagerType::ALGEBRAIC | Moose::RelationshipManagerType::GEOMETRIC;
  params.set<std::string>("for_whom") = "thm_app";
  params.set<MooseMesh *>("mesh") = thm_app.getTHMMesh().get();
  params.set<const std::map<dof_id_type, std::vector<dof_id_type>> *>("_elem_map") =
      &thm_app.getSparsityAugmentationMap();
  auto rm = _factory.create<RelationshipManager>(class_name, "thm:sparsity_btw_elems", params);
  if (!thm_app.addRelationshipManager(rm))
    _factory.releaseSharedObjects(*rm);
}
