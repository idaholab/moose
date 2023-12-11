//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ProjectedStatefulMaterialStorageAction.h"
#include "RankFourTensorForward.h"
#include "RankTwoTensorForward.h"
#include "AddAuxVariableAction.h"
#include "AddVariableAction.h"
#include "FEProblemBase.h"
#include "Factory.h"

#include "libmesh/string_to_enum.h"

// we need to add these variables after the material property types are known
registerMooseAction("MooseApp",
                    ProjectedStatefulMaterialStorageAction,
                    "setup_projected_properties");

registerMooseAction("MooseApp", ProjectedStatefulMaterialStorageAction, "add_aux_kernel");

InputParameters
ProjectedStatefulMaterialStorageAction::validParams()
{
  InputParameters params = Action::validParams();

  params.addClassDescription("Mark material properties for projected stateful storage.");

  params.addParam<MooseEnum>(
      "family",
      MooseEnum("LAGRANGE MONOMIAL L2_LAGRANGE L2_HIERARCHIC", "LAGRANGE"),
      "Finite element variable family to project the material properties onto");
  params.addParam<MooseEnum>(
      "order",
      AddAuxVariableAction::getAuxVariableOrders(),
      "Finite element variable order to project the material properties onto");

  // block restrictions
  params += BlockRestrictable::validParams();

  params.addParam<std::vector<MaterialPropertyName>>(
      "projected_props", {}, "Material properties to project for stateful storage");
  return params;
}

ProjectedStatefulMaterialStorageAction::ProjectedStatefulMaterialStorageAction(
    const InputParameters & params)
  : Action(params),
    _prop_names(getParam<std::vector<MaterialPropertyName>>("projected_props")),
    _order(params.get<MooseEnum>("order")),
    _fe_type({Utility::string_to_enum<Order>(_order),
              Utility::string_to_enum<FEFamily>(params.get<MooseEnum>("family"))}),
    _var_type(AddVariableAction::variableType(_fe_type))
{
}

void
ProjectedStatefulMaterialStorageAction::addRelationshipManagers(
    Moose::RelationshipManagerType input_rm_type)
{
  auto params = ProjectedStatefulMaterialNodalPatchRecoveryBase::validParams();
  addRelationshipManagers(input_rm_type, params);
}

void
ProjectedStatefulMaterialStorageAction::act()
{
  for (const auto & prop_name : _prop_names)
  {
    // loop over all supported property types
    Moose::typeLoop<ProcessProperty>(SupportedTypes{}, this, prop_name);
  }
}

MooseEnum
ProjectedStatefulMaterialStorageAction::getTypeEnum()
{
  return getTypeEnum(SupportedTypes{});
}
