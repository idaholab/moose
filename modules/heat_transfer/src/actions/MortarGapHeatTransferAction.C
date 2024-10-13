//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarGapHeatTransferAction.h"

#include "AddVariableAction.h"
#include "FEProblem.h"
#include "libmesh/string_to_enum.h"
#include "NonlinearSystem.h"

#include "ModularGapConductanceConstraint.h"
#include "GapFluxModelRadiation.h"
#include "GapFluxModelConduction.h"

#include <algorithm>

registerMooseAction("HeatTransferApp", MortarGapHeatTransferAction, "append_mesh_generator");
registerMooseAction("HeatTransferApp", MortarGapHeatTransferAction, "add_mortar_variable");
registerMooseAction("HeatTransferApp", MortarGapHeatTransferAction, "add_constraint");
registerMooseAction("HeatTransferApp", MortarGapHeatTransferAction, "add_user_object");

InputParameters
MortarGapHeatTransferAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Action that controls the creation of all of the necessary objects for "
      "calculation of heat transfer through an open/closed gap using a mortar formulation and a "
      "modular design approach");

  params.addParam<Real>("thermal_lm_scaling",
                        1.,
                        "Scaling factor to apply to the thermal Lagrange multiplier variable");

  params += ModularGapConductanceConstraint::validParams();
  params += GapFluxModelRadiation::validParams();
  params += GapFluxModelConduction::validParams();

  params.addParam<bool>(
      "correct_edge_dropping",
      true,
      "Whether to enable correct edge dropping treatment for mortar constraints. When disabled "
      "any Lagrange Multiplier degree of freedom on a secondary element without full primary "
      "contributions will be set (strongly) to 0.");

  params.makeParamNotRequired<SubdomainName>("primary_subdomain");
  params.makeParamNotRequired<SubdomainName>("secondary_subdomain");
  params.makeParamNotRequired<Real>("gap_conductivity");

  params.addParam<MultiMooseEnum>(
      "gap_flux_options", MortarGapHeatTransfer::gapFluxPhysics, "The gap flux models to build");

  params.addParam<std::vector<UserObjectName>>(
      "user_created_gap_flux_models",
      {},
      "The name of the user objects created by the user to represent gap heat transfer physics");

  params.addParamNamesToGroup("primary_subdomain secondary_subdomain", "Gap surface definition");
  params.addParamNamesToGroup("gap_flux_options user_created_gap_flux_models", "Gap flux models");
  params.addParamNamesToGroup("thermal_lm_scaling correct_edge_dropping",
                              "Thermal Lagrange multiplier");

  return params;
}

MortarGapHeatTransferAction::MortarGapHeatTransferAction(const InputParameters & params)
  : Action(params),
    _user_provided_mortar_meshes(false),
    _user_provided_gap_flux_models(
        getParam<std::vector<UserObjectName>>("user_created_gap_flux_models").size() > 0 ? true
                                                                                         : false)

{
  if (getParam<MultiMooseEnum>("gap_flux_options").size() > 0 && _user_provided_gap_flux_models)
    paramError(
        "gap_flux_options",
        "Either create user objects for the action in the input file or provide the desire physics "
        "to the action via the gap_flux_options parameter. Mixed use is not supported");

  for (unsigned int i = 0; i < getParam<MultiMooseEnum>("gap_flux_options").size(); i++)
    _gap_flux_models.push_back(static_cast<MortarGapHeatTransfer::UserObjectToBuild>(
        getParam<MultiMooseEnum>("gap_flux_options").get(i)));

  // We do not currently support building more than one condution or more than one radiation user
  // object from this action.
  const unsigned int conduction_build_uos =
      cast_int<unsigned int>(std::count(_gap_flux_models.cbegin(),
                                        _gap_flux_models.cend(),
                                        MortarGapHeatTransfer::UserObjectToBuild::CONDUCTION));
  const unsigned int radiation_build_uos =
      cast_int<unsigned int>(std::count(_gap_flux_models.cbegin(),
                                        _gap_flux_models.cend(),
                                        MortarGapHeatTransfer::UserObjectToBuild::RADIATION));

  if (conduction_build_uos > 1 || radiation_build_uos > 1)
    paramError("gap_flux_options",
               "You cannot choose to have more than one conduction or more than one radiation user "
               "objects when they are built by the action. If you want to superimpose multiple "
               "physics, you can choose to create your own user objects and pass them to this "
               "action via 'user_created_gap_flux_models'");

  if (params.isParamSetByUser("primary_subdomain") &&
      params.isParamSetByUser("secondary_subdomain"))
  {
    mooseInfo("Mortar gap heat transfer action is using the lower-dimensional domains provided by "
              "the user");
    _user_provided_mortar_meshes = true;
  }
  else
    mooseInfo("Mortar gap heat transfer action is creating new lower-dimensional domains");

  if (_user_provided_gap_flux_models)
    mooseInfo(
        "User decided to create user objects to model physics for the mortar gap heat transfer "
        "action independently, i.e. not through the mortar gap heat transfer action.");
  else
    mooseInfo("The mortar gap heat transfer action will add gap heat transfer physics according to "
              "the gap_flux_options input parameter");

  const bool wrong_parameters_provided =
      _user_provided_gap_flux_models &&
      (params.isParamSetByUser("gap_conductivity") ||
       params.isParamSetByUser("primary_emissivity") ||
       params.isParamSetByUser("secondary_emissivity") ||
       params.isParamSetByUser("gap_conductivity_function") ||
       params.isParamSetByUser("gap_conductivity_function_variable") ||
       params.isParamSetByUser("min_gap"));

  if (wrong_parameters_provided)
    paramError(
        "user_created_gap_flux_models",
        "The mortar gap heat transfer action requires that the input file defines user objects "
        "with physics or adds physics parameters directly into the action. You have provided both "
        "user objects and physics parameters (e.g. emissivities, gap conductance, etc.).");
}

void
MortarGapHeatTransferAction::act()
{
  if (_current_task == "append_mesh_generator")
    addMortarMesh();
  else if (_current_task == "add_mortar_variable")
    addMortarVariable();
  if (_current_task == "add_constraint")
    addConstraints();
  else if (_current_task == "add_user_object")
    if (!_user_provided_gap_flux_models)
      addUserObjects();
}

void
MortarGapHeatTransferAction::coreMortarMesh()
{
  if (!(_app.isRecovering() && _app.isUltimateMaster()) && !_app.masterMesh())
  {
    std::string action_name = MooseUtils::shortName(name());

    const MeshGeneratorName primary_name = action_name + "_primary_subdomain" + "_generator";
    const MeshGeneratorName secondary_name = action_name + "_secondary_subdomain" + "_generator";

    auto primary_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");
    auto secondary_params = _factory.getValidParams("LowerDBlockFromSidesetGenerator");

    primary_params.set<SubdomainName>("new_block_name") = action_name + "_primary_subdomain";
    secondary_params.set<SubdomainName>("new_block_name") = action_name + "_secondary_subdomain";

    primary_params.set<std::vector<BoundaryName>>("sidesets") = {
        getParam<BoundaryName>("primary_boundary")};
    secondary_params.set<std::vector<BoundaryName>>("sidesets") = {
        getParam<BoundaryName>("secondary_boundary")};

    _app.appendMeshGenerator("LowerDBlockFromSidesetGenerator", primary_name, primary_params);
    _app.appendMeshGenerator("LowerDBlockFromSidesetGenerator", secondary_name, secondary_params);
  }
}

void
MortarGapHeatTransferAction::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  checkForExistingSubdomains();
  std::string action_name = MooseUtils::shortName(name());

  auto params = MortarConstraintBase::validParams();
  params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
  params.set<BoundaryName>("primary_boundary") = getParam<BoundaryName>("primary_boundary");
  params.set<BoundaryName>("secondary_boundary") = getParam<BoundaryName>("secondary_boundary");

  if (_user_provided_mortar_meshes)
  {
    params.set<SubdomainName>("primary_subdomain") = getParam<SubdomainName>("primary_subdomain");
    params.set<SubdomainName>("secondary_subdomain") =
        getParam<SubdomainName>("secondary_subdomain");
  }
  else
  {
    params.set<SubdomainName>("primary_subdomain") = action_name + "_primary_subdomain";
    params.set<SubdomainName>("secondary_subdomain") = action_name + "_secondary_subdomain";
  }

  addRelationshipManagers(input_rm_type, params);
}

void
MortarGapHeatTransferAction::addMortarVariable()
{
  checkForExistingSubdomains();

  InputParameters params = _factory.getValidParams("MooseVariableBase");

  const std::string & temperature = getParam<std::vector<VariableName>>("temperature")[0];
  std::string action_name = MooseUtils::shortName(name());

  if (!_problem->hasVariable(temperature))
    mooseError("Temperature variable is missing");

  const auto primal_type =
      _problem->getVariable(0, temperature, Moose::VarKindType::VAR_SOLVER).feType();
  const int lm_order = primal_type.order.get_order();

  if (primal_type.family != LAGRANGE)
    mooseError("The mortar thermal action can only be used with LAGRANGE finite elements");

  params.set<MooseEnum>("family") = Utility::enum_to_string<FEFamily>(primal_type.family);
  params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(libMesh::OrderWrapper{lm_order});

  if (_user_provided_mortar_meshes)
    params.set<std::vector<SubdomainName>>("block") = {
        getParam<SubdomainName>("secondary_subdomain")};
  else
    params.set<std::vector<SubdomainName>>("block") = {action_name + "_secondary_subdomain"};

  params.set<std::vector<Real>>("scaling") = {getParam<Real>("thermal_lm_scaling")};
  auto fe_type = AddVariableAction::feType(params);
  auto var_type = AddVariableAction::variableType(fe_type);

  _problem->addVariable(var_type, action_name + "_thermal_lm", params);
}

void
MortarGapHeatTransferAction::addConstraints()
{
  checkForExistingSubdomains();

  InputParameters params = _factory.getValidParams("ModularGapConductanceConstraint");
  const std::string action_name = MooseUtils::shortName(name());

  params.applyParameters(parameters());
  params.set<bool>("use_displaced_mesh") = true;

  params.set<BoundaryName>("primary_boundary") = getParam<BoundaryName>("primary_boundary");
  params.set<BoundaryName>("secondary_boundary") = getParam<BoundaryName>("secondary_boundary");

  if (_user_provided_mortar_meshes)
  {
    params.set<SubdomainName>("primary_subdomain") = getParam<SubdomainName>("primary_subdomain");
    params.set<SubdomainName>("secondary_subdomain") =
        getParam<SubdomainName>("secondary_subdomain");
  }
  else
  {
    params.set<SubdomainName>("primary_subdomain") = action_name + "_primary_subdomain";
    params.set<SubdomainName>("secondary_subdomain") = action_name + "_secondary_subdomain";
  }

  params.set<NonlinearVariableName>("variable") = action_name + "_thermal_lm";

  params.set<VariableName>("secondary_variable") =
      getParam<std::vector<VariableName>>("temperature")[0];

  if (!_user_provided_gap_flux_models)
  {
    std::vector<UserObjectName> uoname_strings(0);

    for (const auto & uo_name : _gap_flux_models)
    {
      if (uo_name == MortarGapHeatTransfer::UserObjectToBuild::CONDUCTION)
        uoname_strings.push_back("gap_flux_model_conduction_object_" +
                                 MooseUtils::shortName(name()));
      else if (uo_name == MortarGapHeatTransfer::UserObjectToBuild::RADIATION)
        uoname_strings.push_back("gap_flux_model_radiation_object_" +
                                 MooseUtils::shortName(name()));
    }

    params.set<std::vector<UserObjectName>>("gap_flux_models") = uoname_strings;
  }
  else
    params.set<std::vector<UserObjectName>>("gap_flux_models") =
        getParam<std::vector<UserObjectName>>("user_created_gap_flux_models");

  _problem->addConstraint(
      "ModularGapConductanceConstraint", action_name + "_ModularGapConductanceConstraint", params);
}

void
MortarGapHeatTransferAction::addMortarMesh()
{
  // Evaluate whether we have sufficient information from the user to skip building the
  // lower-dimensional domains.
  checkForExistingSubdomains();

  // We may have available lower-dimensional domains (e.g. from a mechanical contact action), whose
  // subdomains can be reused for adding mortar variables and constraints.
  if (!_user_provided_mortar_meshes)
    coreMortarMesh();
}

void
MortarGapHeatTransferAction::addUserObjects()
{
  // It is risky to apply this optimization to contact problems
  // since the problem configuration may be changed during Jacobian
  // evaluation. We therefore turn it off for all contact problems so that
  // PETSc-3.8.4 or higher will have the same behavior as PETSc-3.8.3 or older.
  mooseAssert(_problem, "Problem pointer is null");

  if (!_problem->isSNESMFReuseBaseSetbyUser())
    _problem->setSNESMFReuseBase(false, false);

  for (const auto & uo_name : _gap_flux_models)
  {
    if (uo_name == MortarGapHeatTransfer::UserObjectToBuild::CONDUCTION)
    {
      auto var_params = _factory.getValidParams("GapFluxModelConduction");

      var_params.set<std::vector<VariableName>>("temperature") =
          getParam<std::vector<VariableName>>("temperature");
      var_params.set<Real>("gap_conductivity") = getParam<Real>("gap_conductivity");

      if (isParamValid("gap_conductivity_function"))
        var_params.set<FunctionName>("gap_conductivity_function") =
            getParam<FunctionName>("gap_conductivity_function");

      if (isParamValid("gap_conductivity_function_variable"))
        var_params.set<std::vector<VariableName>>("gap_conductivity_function_variable") =
            getParam<std::vector<VariableName>>("gap_conductivity_function_variable");

      var_params.set<Real>("min_gap") = getParam<Real>("min_gap");
      var_params.set<unsigned int>("min_gap_order") = getParam<unsigned int>("min_gap_order");

      var_params.set<std::vector<BoundaryName>>("boundary") =
          getParam<std::vector<BoundaryName>>("boundary");

      var_params.set<bool>("use_displaced_mesh") = true;

      _problem->addUserObject("GapFluxModelConduction",
                              "gap_flux_model_conduction_object_" + MooseUtils::shortName(name()),
                              var_params);
    }
    else if (uo_name == MortarGapHeatTransfer::UserObjectToBuild::RADIATION)

    {
      auto var_params = _factory.getValidParams("GapFluxModelRadiation");

      var_params.set<Real>("stefan_boltzmann") = getParam<Real>("stefan_boltzmann");
      var_params.set<Real>("primary_emissivity") = getParam<Real>("primary_emissivity");
      var_params.set<Real>("secondary_emissivity") = getParam<Real>("secondary_emissivity");

      var_params.set<std::vector<BoundaryName>>("boundary") =
          getParam<std::vector<BoundaryName>>("boundary");

      var_params.set<std::vector<VariableName>>("temperature") =
          getParam<std::vector<VariableName>>("temperature");

      var_params.set<bool>("use_displaced_mesh") = true;

      _problem->addUserObject("GapFluxModelRadiation",
                              "gap_flux_model_radiation_object_" + MooseUtils::shortName(name()),
                              var_params);
    }
  }
}

void
MortarGapHeatTransferAction::checkForExistingSubdomains()
{
  if (parameters().isParamSetByUser("primary_subdomain") &&
      parameters().isParamSetByUser("secondary_subdomain"))
  {
    _user_provided_mortar_meshes = true;
  }
  else
    _user_provided_mortar_meshes = false;
}
