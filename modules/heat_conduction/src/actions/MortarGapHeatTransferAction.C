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

// Counter for modular user objects
static unsigned int thermal_action_userobject_radiation_counter = 0;
static unsigned int thermal_action_userobject_conduction_counter = 0;

registerMooseAction("HeatConductionApp", MortarGapHeatTransferAction, "append_mesh_generator");
registerMooseAction("HeatConductionApp", MortarGapHeatTransferAction, "add_mortar_variable");
registerMooseAction("HeatConductionApp", MortarGapHeatTransferAction, "add_constraint");
registerMooseAction("HeatConductionApp", MortarGapHeatTransferAction, "add_user_object");

InputParameters
MortarGapHeatTransferAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Action that controls the creation of all of the necessary objects for "
      "calculation of heat transfer through an open/closed gap");

  // Modular mortar gap conductance
  MooseEnum gap_heat_transfer_formulation("point_segment mortar", "mortar");
  params.addParam<MooseEnum>(
      "formulation", gap_heat_transfer_formulation, "The gap heat transfer formulation to be used");

  params.addParam<Real>("thermal_lm_scaling",
                        1.,
                        "Scaling factor to apply to the thermal Lagrange multiplier variable");

  params += ModularGapConductanceConstraint::validParams();
  params += GapFluxModelRadiation::validParams();
  params += GapFluxModelConduction::validParams();

  params.makeParamNotRequired<SubdomainName>("primary_subdomain");
  params.makeParamNotRequired<SubdomainName>("secondary_subdomain");

  params.addParam<std::vector<UserObjectName>>(
      "user_object_physics",
      "The list of physical contributions to gap heat transfer implemented in user objects");

  return params;
}

MortarGapHeatTransferAction::MortarGapHeatTransferAction(const InputParameters & params)
  : Action(params),
    _formulation(getParam<MooseEnum>("formulation").getEnum<GapHeatTransferFormulation>()),
    _user_provided_mortar_meshes(false)
{

  if (params.isParamSetByUser("primary_subdomain") &&
      params.isParamSetByUser("secondary_subdomain"))
  {
    mooseInfo("Mortar gap heat transfer action is using the lower-dimensional domains provided by "
              "the user");
    _user_provided_mortar_meshes = true;
  }
  else
    mooseInfo("Mortar gap heat transfer action is creating new lower-dimensional domains");
}

void
MortarGapHeatTransferAction::act()
{
  if (_formulation == GapHeatTransferFormulation::MORTAR)
  {

    if (_current_task == "append_mesh_generator")
      addMortarMesh();
    else if (_current_task == "add_mortar_variable")
      addMortarVariable();
    if (_current_task == "add_constraint")
      addConstraints();
    else if (_current_task == "add_user_object")
      addUserObjects();
  }
  else
    paramError(
        "formulation",
        "The formulation selected to solve gap heat transfer physics is not currently available.");
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

  mooseAssert(_problem->systemBaseNonlinear().hasVariable(temperature),
              "Temperature variable is missing");

  const auto primal_type = _problem->systemBaseNonlinear().system().variable_type(temperature);
  const int lm_order = primal_type.order.get_order();

  if (primal_type.family != LAGRANGE)
    mooseError("The mortar thermal action can only be used with LAGRANGE finite elements");

  params.set<MooseEnum>("family") = Utility::enum_to_string<FEFamily>(primal_type.family);
  params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{lm_order});

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
  params.set<bool>("correct_edge_dropping") = true;
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

  const std::vector<UserObjectName> user_object_vector =
      getParam<std::vector<UserObjectName>>("user_object_physics");

  std::vector<UserObjectName> uoname_strings(0);
  unsigned int conduction_index = 0;
  unsigned int radiation_index = 0;

  for (const auto i : index_range(user_object_vector))
  {
    if (user_object_vector[i] == "GapFluxModelConduction")
      uoname_strings.push_back("gap_flux_model_conduction_object_" + MooseUtils::shortName(name()) +
                               "_" + Moose::stringify(conduction_index++));
    else if (user_object_vector[i] == "GapFluxModelRadiation")
      uoname_strings.push_back("gap_flux_model_radiation_object_" + MooseUtils::shortName(name()) +
                               "_" + Moose::stringify(radiation_index++));
  }

  params.set<std::vector<UserObjectName>>("gap_flux_models") = uoname_strings;
  _problem->addConstraint(
      "ModularGapConductanceConstraint", action_name + "_ModularGapConductanceConstraint", params);
}

void
MortarGapHeatTransferAction::addMortarMesh()
{
  // Let's browse over existing mechanical actions to see if the primary and secondary
  // subdomains have been created
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

  const std::vector<UserObjectName> user_object_vector =
      getParam<std::vector<UserObjectName>>("user_object_physics");

  for (const auto i : index_range(user_object_vector))
  {
    if (user_object_vector[i] == "GapFluxModelConduction")
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
                              "gap_flux_model_conduction_object_" + MooseUtils::shortName(name()) +
                                  "_" +
                                  Moose::stringify(thermal_action_userobject_conduction_counter++),
                              var_params);
    }
    else if (user_object_vector[i] == "GapFluxModelRadiation")
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
                              "gap_flux_model_radiation_object_" + MooseUtils::shortName(name()) +
                                  "_" +
                                  Moose::stringify(thermal_action_userobject_radiation_counter++),
                              var_params);
    }
    else
      paramError("user_object_physics",
                 "At least one of the user objects provided to capture gap heat transfer physics "
                 "using a mortar formulation are not currently supported");
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
