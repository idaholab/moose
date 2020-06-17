//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalContactAction.h"

#include "AddVariableAction.h"
#include "FEProblem.h"
#include "libmesh/string_to_enum.h"
#include "GapConductance.h"
#include "GapConductanceConstant.h"
#include "NonlinearSystem.h"

registerMooseAction("HeatConductionApp", ThermalContactAction, "add_aux_kernel");
registerMooseAction("HeatConductionApp", ThermalContactAction, "add_aux_variable");
registerMooseAction("HeatConductionApp", ThermalContactAction, "add_bc");
registerMooseAction("HeatConductionApp", ThermalContactAction, "add_dirac_kernel");
registerMooseAction("HeatConductionApp", ThermalContactAction, "add_material");
registerMooseAction("HeatConductionApp", ThermalContactAction, "add_secondary_flux_vector");

InputParameters
ThermalContactAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Action that controls the creation of all of the necessary objects for "
      "calculation of Thermal Contact");

  params.addParam<std::string>(
      "gap_aux_type",
      "GapValueAux",
      "A string representing the Moose object that will be used for computing the gap size");
  params.addRequiredParam<NonlinearVariableName>("variable", "The variable for thermal contact");
  params.addRequiredParam<BoundaryName>("primary", "The primary surface");
  params.addRequiredParam<BoundaryName>("secondary", "The secondary surface");
  params.addRangeCheckedParam<Real>("tangential_tolerance",
                                    "tangential_tolerance>=0",
                                    "Tangential distance to extend edges of contact surfaces");
  params.addRangeCheckedParam<Real>(
      "normal_smoothing_distance",
      "normal_smoothing_distance>=0 & normal_smoothing_distance<=1",
      "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method",
                               "Method to use to smooth normals (edge_based|nodal_normal_based)");

  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>("order", orders, "The finite element order");

  params.addParam<bool>(
      "warnings", false, "Whether to output warning messages concerning nodes not being found");
  params.addParam<bool>(
      "quadrature", false, "Whether or not to use quadrature point based gap heat transfer");

  params.addParam<std::string>(
      "appended_property_name", "", "Name appended to material properties to make them unique");
  params.addRequiredParam<std::string>(
      "type",
      "A string representing the Moose object that will be used for heat conduction over the gap");

  params.addParam<std::vector<VariableName>>("disp_x", "The x displacement");
  params.addParam<std::vector<VariableName>>("disp_y", "The y displacement");
  params.addParam<std::vector<VariableName>>("disp_z", "The z displacement");
  params.addParam<std::vector<VariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

  params.addParam<std::vector<AuxVariableName>>(
      "save_in", "The Auxiliary Variable to (optionally) save the boundary flux in");
  params.addRangeCheckedParam<Real>("gap_conductivity",
                                    1.0,
                                    "gap_conductivity>0",
                                    "The thermal conductivity of the gap material");
  params.addParam<FunctionName>(
      "gap_conductivity_function",
      "Thermal conductivity of the gap material as a function.  Multiplied by gap_conductivity.");
  params.addParam<std::vector<VariableName>>(
      "gap_conductivity_function_variable",
      "Variable to be used in gap_conductivity_function in place of time");

  params += GapConductance::actionParameters();
  params += GapConductanceConstant::actionParameters();

  return params;
}

ThermalContactAction::ThermalContactAction(const InputParameters & params)
  : Action(params),
    _quadrature(getParam<bool>("quadrature")),
    _order(getParam<MooseEnum>("order")),
    _penetration_var_name(_quadrature ? "qpoint_penetration" : "penetration"),
    _gap_value_name("paired_" + getParam<NonlinearVariableName>("variable")),
    _gap_conductivity_name("paired_k_" + getParam<NonlinearVariableName>("variable"))
{
}

void
ThermalContactAction::act()
{
  if (_current_task == "add_aux_kernel")
    addAuxKernels();
  else if (_current_task == "add_aux_variable")
    addAuxVariables();
  else if (_current_task == "add_bc")
    addBCs();
  else if (_current_task == "add_dirac_kernel")
    addDiracKernels();
  else if (_current_task == "add_material")
    addMaterials();
  else if (_current_task == "add_secondary_flux_vector")
    addSlaveFluxVector();
}

void
ThermalContactAction::addAuxKernels()
{
  // Add gap aux kernel
  {
    InputParameters params = _factory.getValidParams(getParam<std::string>("gap_aux_type"));

    params.applySpecificParameters(parameters(),
                                   {"tangential_tolerance",
                                    "normal_smoothing_distance",
                                    "normal_smoothing_method",
                                    "order",
                                    "warnings"});
    params.set<AuxVariableName>("variable") = _gap_value_name;
    params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_LINEAR};
    params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("secondary")};
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("primary");
    params.set<VariableName>("paired_variable") = getParam<NonlinearVariableName>("variable");

    _problem->addAuxKernel(getParam<std::string>("gap_aux_type"), "gap_value_" + name(), params);

    if (_quadrature)
    {
      params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("primary")};
      params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("secondary");

      _problem->addAuxKernel(
          getParam<std::string>("gap_aux_type"), "gap_value_primary_" + name(), params);
    }
  }

  // Add penetration aux kernel
  {
    InputParameters params = _factory.getValidParams("PenetrationAux");

    params.applySpecificParameters(
        parameters(),
        {"tangential_tolerance", "normal_smoothing_distance", "normal_smoothing_method", "order"});
    params.set<AuxVariableName>("variable") = _penetration_var_name;
    params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_LINEAR};
    params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("secondary")};
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("primary");

    _problem->addAuxKernel("PenetrationAux", "penetration_" + name(), params);
  }
}

void
ThermalContactAction::addAuxVariables()
{
  // We need to add variables only once per variable name.  However, we don't know how many unique
  // variable names we will have.  So, we'll always add them.

  MooseEnum order = getParam<MooseEnum>("order");
  std::string family = "LAGRANGE";

  if (_quadrature)
  {
    order = "CONSTANT";
    family = "MONOMIAL";
  }

  auto var_type =
      AddVariableAction::determineType(FEType(order, Utility::string_to_enum<FEFamily>(family)), 1);
  auto var_params = _factory.getValidParams(var_type);
  var_params.set<MooseEnum>("order") = order;
  var_params.set<MooseEnum>("family") = family;

  _problem->addAuxVariable(var_type, _penetration_var_name, var_params);
  _problem->addAuxVariable(var_type, _gap_value_name, var_params);
}

void
ThermalContactAction::addBCs()
{
  const std::string object_name = getParam<std::string>("type");
  InputParameters params = _factory.getValidParams(object_name);
  params.applyParameters(parameters());

  if (_quadrature)
  {
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("primary");
    params.set<bool>("use_displaced_mesh") = true;
  }
  else
  {
    params.set<std::vector<VariableName>>("gap_distance") = {"penetration"};
    params.set<std::vector<VariableName>>("gap_temp") = {_gap_value_name};
  }

  params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("secondary")};

  _problem->addBoundaryCondition(object_name, "gap_bc_" + name(), params);

  if (_quadrature)
  {
    // Swap primary and secondary for this one
    params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("primary")};
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("secondary");

    _problem->addBoundaryCondition(object_name, "gap_bc_primary_" + name(), params);
  }
}

void
ThermalContactAction::addDiracKernels()
{
  if (_quadrature)
    return;

  const std::string object_name = "GapHeatPointSourceMaster";
  InputParameters params = _factory.getValidParams(object_name);
  params.applySpecificParameters(parameters(),
                                 {"tangential_tolerance",
                                  "normal_smoothing_distance",
                                  "normal_smoothing_method",
                                  "order",
                                  "secondary",
                                  "variable"});
  params.set<BoundaryName>("boundary") = getParam<BoundaryName>("primary");

  _problem->addDiracKernel(object_name, object_name + "_" + name(), params);
}

void
ThermalContactAction::addMaterials()
{
  if (getParam<std::string>("type") != "GapHeatTransfer")
    return;

  if (parameters().isParamSetByUser("gap_conductance"))
  {
    if (parameters().isParamSetByUser("gap_conductivity") ||
        parameters().isParamSetByUser("gap_conductivity_function"))
      mooseError(
          "Cannot specify both gap_conductance and gap_conductivity or gap_conductivity_function");

    const std::string object_type = "GapConductanceConstant";
    InputParameters params = _factory.getValidParams(object_type);
    params.applyParameters(parameters());
    params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("secondary")};
    _problem->addMaterial(object_type, name() + "_" + "gap_value", params);

    if (_quadrature)
    {
      params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("primary")};
      _problem->addMaterial(object_type, name() + "_" + "gap_value_primary", params);
    }
  }
  else
  {
    const std::string object_type = "GapConductance";

    InputParameters params = _factory.getValidParams(object_type);
    params.applyParameters(parameters(), {"variable"});

    params.set<std::vector<VariableName>>("variable") = {
        getParam<NonlinearVariableName>("variable")};
    params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("secondary")};

    if (_quadrature)
    {
      params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("primary");
    }
    else
    {
      params.set<std::vector<VariableName>>("gap_temp") = {_gap_value_name};
      params.set<std::vector<VariableName>>("gap_distance") = {"penetration"};
    }

    _problem->addMaterial(object_type, name() + "_" + "gap_value", params);

    if (_quadrature)
    {
      params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("secondary");
      params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("primary")};

      _problem->addMaterial(object_type, name() + "_" + "gap_value_primary", params);
    }
  }
}

void
ThermalContactAction::addSlaveFluxVector()
{
  _problem->getNonlinearSystemBase().addVector("secondary_flux", false, GHOSTED);
  _problem->getNonlinearSystemBase().zeroVectorForResidual("secondary_flux");

  // It is risky to apply this optimization to contact problems
  // since the problem configuration may be changed during Jacobian
  // evaluation. We therefore turn it off for all contact problems so that
  // PETSc-3.8.4 or higher will have the same behavior as PETSc-3.8.3 or older.
  if (!_problem->isSNESMFReuseBaseSetbyUser())
    _problem->setSNESMFReuseBase(false, false);
}
