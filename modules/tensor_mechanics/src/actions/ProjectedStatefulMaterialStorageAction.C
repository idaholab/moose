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
#include "Conversion.h"
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
      MooseEnum("LAGRANGE MONOMIAL", "LAGRANGE"),
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
    _var_type(AddVariableAction::variableType(_fe_type)),
    _pomps_prefix("_pomps_")
{
}

void
ProjectedStatefulMaterialStorageAction::processComponent(const std::string & prop_name,
                                                         std::vector<unsigned int> idx,
                                                         std::vector<VariableName> & vars,
                                                         bool is_ad)
{
  const std::string pomps_prefix = "_pomps_";
  auto name = [&](const std::string & type)
  {
    auto name = _pomps_prefix + type + "_" + prop_name;
    if (idx.size())
      name += "_" + Moose::stringify(idx, "_");
    return name;
  };

  const auto var_name = name("var");
  vars.push_back(var_name);

  if (_current_task == "setup_projected_properties")
  {
    // add the AuxVars for storage
    auto params = _factory.getValidParams(_var_type);
    params.applyParameters(parameters());
    params.set<std::vector<OutputName>>("outputs") = {"none"};
    _problem->addAuxVariable(_var_type, var_name, params);
  }

  if (_current_task == "add_aux_kernel")
  {
    // use nodal patch recovery for lagrange
    if (_fe_type.family == LAGRANGE)
    {
      // nodal variables require patch recovery
      const auto uo_name = name("uo");

      {
        // add user object
        const auto & type_name = is_ad ? "ADProjectedStatefulMaterialNodalPatchRecovery"
                                       : "ProjectedStatefulMaterialNodalPatchRecovery";
        auto params = _factory.getValidParams(type_name);
        params.applySpecificParameters(parameters(), {"block"});
        params.set<std::vector<unsigned int>>("component") = idx;
        params.set<MaterialPropertyName>("property") = prop_name;
        params.set<MooseEnum>("patch_polynomial_order") = _order;
        params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
        _problem->addUserObject(type_name, uo_name, params);
      }

      {
        // add aux kernel
        auto params = _factory.getValidParams("NodalPatchRecoveryAux");
        params.applySpecificParameters(parameters(), {"block"});
        params.set<AuxVariableName>("variable") = var_name;
        params.set<UserObjectName>("nodal_patch_recovery_uo") = uo_name;
        params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
        _problem->addAuxKernel("NodalPatchRecoveryAux", name("aux"), params);
      }
    }
    else
    {
      // elemental variables
      {
        const auto & type_name =
            is_ad ? "ADProjectedStatefulMaterialAux" : "ProjectedStatefulMaterialAux";
        auto params = _factory.getValidParams(type_name);
        params.applySpecificParameters(parameters(), {"block"});
        params.set<AuxVariableName>("variable") = var_name;
        params.set<std::vector<unsigned int>>("component") = idx;
        params.set<MaterialPropertyName>("property") = prop_name;
        params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
        _problem->addAuxKernel(type_name, name("aux"), params);
      }
    }
  }
}

ProjectedStatefulMaterialStorageAction::PropertyInfo
ProjectedStatefulMaterialStorageAction::checkProperty(const std::string & prop_name)
{
  const auto & data = _problem->getMaterialData(Moose::BLOCK_MATERIAL_DATA);

  if (data.haveProperty<Real>(prop_name))
    return {PropertyType::REAL, false};
  if (data.haveADProperty<Real>(prop_name))
    return {PropertyType::REAL, true};

  if (data.haveProperty<RealVectorValue>(prop_name))
    return {PropertyType::REALVECTORVALUE, false};
  if (data.haveADProperty<RealVectorValue>(prop_name))
    return {PropertyType::REALVECTORVALUE, true};

  if (data.haveProperty<RankTwoTensor>(prop_name))
    return {PropertyType::RANKTWOTENSOR, false};
  if (data.haveADProperty<RankTwoTensor>(prop_name))
    return {PropertyType::RANKTWOTENSOR, true};

  if (data.haveProperty<RankFourTensor>(prop_name))
    return {PropertyType::RANKFOURTENSOR, false};
  if (data.haveADProperty<RankFourTensor>(prop_name))
    return {PropertyType::RANKFOURTENSOR, true};

  paramError("projected_props",
             "Material property type of property '",
             prop_name,
             "' is not supported for projection.");
}

void
ProjectedStatefulMaterialStorageAction::addMaterial(const std::string & prop_type,
                                                    const std::string & prop_name,
                                                    std::vector<VariableName> & vars)
{
  auto params = _factory.getValidParams("InterpolatedStatefulMaterial");
  const auto name = _pomps_prefix + "mat_" + prop_name;
  params.applySpecificParameters(parameters(), {"block"});
  params.set<std::vector<VariableName>>("old_state") = vars;
  params.set<MooseEnum>("prop_type") = prop_type;
  params.set<MaterialPropertyName>("prop_name") = name;
  _problem->addMaterial("InterpolatedStatefulMaterial", name, params);
}

void
ProjectedStatefulMaterialStorageAction::act()
{
  for (const auto & prop_name : _prop_names)
  {
    const auto prop_info = checkProperty(prop_name);
    std::vector<VariableName> vars;

    switch (prop_info.first)
    {
      case PropertyType::REAL:
        processComponent(prop_name, {}, vars, prop_info.second);
        if (_current_task == "setup_projected_properties")
          addMaterial("REAL", prop_name, vars);
        break;

      case PropertyType::REALVECTORVALUE:
        for (unsigned int i = 0; i < Moose::dim; ++i)
          processComponent(prop_name, {i}, vars, prop_info.second);
        if (_current_task == "setup_projected_properties")
          addMaterial("REALVECTORVALUE", prop_name, vars);
        break;

      case PropertyType::RANKTWOTENSOR:
        for (unsigned int i = 0; i < Moose::dim; ++i)
          for (unsigned int j = 0; j < Moose::dim; ++j)
            processComponent(prop_name, {i, j}, vars, prop_info.second);
        if (_current_task == "setup_projected_properties")
          addMaterial("RANKTWOTENSOR", prop_name, vars);
        break;

      case PropertyType::RANKFOURTENSOR:
        for (unsigned int i = 0; i < Moose::dim; ++i)
          for (unsigned int j = 0; j < Moose::dim; ++j)
            for (unsigned int k = 0; k < Moose::dim; ++k)
              for (unsigned int l = 0; l < Moose::dim; ++l)
                processComponent(prop_name, {i, j, k, l}, vars, prop_info.second);
        if (_current_task == "setup_projected_properties")
          addMaterial("RANKFOURTENSOR", prop_name, vars);
        break;
    }
  }
}
