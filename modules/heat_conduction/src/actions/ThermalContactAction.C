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
#include "NonlinearSystem.h"

registerMooseAction("HeatConductionApp", ThermalContactAction, "add_aux_kernel");
registerMooseAction("HeatConductionApp", ThermalContactAction, "add_aux_variable");
registerMooseAction("HeatConductionApp", ThermalContactAction, "add_bc");
registerMooseAction("HeatConductionApp", ThermalContactAction, "add_dirac_kernel");
registerMooseAction("HeatConductionApp", ThermalContactAction, "add_material");
registerMooseAction("HeatConductionApp", ThermalContactAction, "add_slave_flux_vector");

template <>
InputParameters
validParams<ThermalContactAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription(
      "Action that controls the creation of all of the necessary objects for "
      "calculation of Thermal Contact");

  params.addParam<std::string>(
      "gap_aux_type",
      "GapValueAux",
      "A string representing the Moose object that will be used for computing the gap size");
  params.addRequiredParam<NonlinearVariableName>("variable", "The variable for thermal contact");
  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>(
      "normal_smoothing_distance",
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
  params.addDeprecatedParam<std::vector<VariableName>>(
      "disp_x", "The x displacement", "Use displacements instead");
  params.addDeprecatedParam<std::vector<VariableName>>(
      "disp_y", "The y displacement", "Use displacements instead");
  params.addDeprecatedParam<std::vector<VariableName>>(
      "disp_z", "The z displacement", "Use displacements instead");
  params.addParam<std::vector<VariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<std::vector<AuxVariableName>>(
      "save_in", "The Auxiliary Variable to (optionally) save the boundary flux in");

  params.addParam<Real>("gap_conductivity", 1.0, "The thermal conductivity of the gap material");
  params.addParam<FunctionName>(
      "gap_conductivity_function",
      "Thermal conductivity of the gap material as a function.  Multiplied by gap_conductivity.");
  params.addParam<std::vector<VariableName>>(
      "gap_conductivity_function_variable",
      "Variable to be used in gap_conductivity_function in place of time");
  params.addParam<std::string>("conductivity_name",
                               "thermal_conductivity",
                               "The name of the MaterialProperty associated with conductivity "
                               "(\"thermal_conductivity\" in the case of heat conduction)");
  params.addParam<std::string>("conductivity_master_name",
                               "thermal_conductivity",
                               "The name of the MaterialProperty associated with conductivity "
                               "(\"thermal_conductivity\" in the case of heat conduction)");

  params += GapConductance::actionParameters();

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
  else if (_current_task == "add_slave_flux_vector")
    addSlaveFluxVector();
}

void
ThermalContactAction::addAuxKernels()
{
  // Add gap aux kernel
  {
    InputParameters params = _factory.getValidParams(getParam<std::string>("gap_aux_type"));

    params.applySpecificParameters(
        parameters(),
        {"tangential_tolerance", "normal_smoothing_distance", "normal_smoothing_method", "order"});
    params.set<AuxVariableName>("variable") = _gap_value_name;
    params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_LINEAR};
    params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("slave")};
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");
    params.set<VariableName>("paired_variable") = getParam<NonlinearVariableName>("variable");

    _problem->addAuxKernel(getParam<std::string>("gap_aux_type"), "gap_value_" + name(), params);

    if (_quadrature)
    {
      params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("master")};
      params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("slave");

      _problem->addAuxKernel(
          getParam<std::string>("gap_aux_type"), "gap_value_master_" + name(), params);
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
    params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("slave")};
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");

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

  _problem->addAuxVariable(_penetration_var_name,
                           FEType(order, Utility::string_to_enum<FEFamily>(family)));

  _problem->addAuxVariable(_gap_value_name,
                           FEType(order, Utility::string_to_enum<FEFamily>(family)));
}

void
ThermalContactAction::addBCs()
{
  const std::string object_name = getParam<std::string>("type");
  InputParameters params = _factory.getValidParams(object_name);
  params.applyParameters(parameters());

  if (_quadrature)
  {
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");
    params.set<bool>("use_displaced_mesh") = true;
  }
  else
  {
    params.set<std::vector<VariableName>>("gap_distance") = {"penetration"};
    params.set<std::vector<VariableName>>("gap_temp") = {_gap_value_name};
  }

  params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("slave")};

  _problem->addBoundaryCondition(object_name, "gap_bc_" + name(), params);

  if (_quadrature)
  {
    // Swap master and slave for this one
    params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("master")};
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("slave");

    _problem->addBoundaryCondition(object_name, "gap_bc_master_" + name(), params);
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
                                  "slave",
                                  "variable"});
  params.set<BoundaryName>("boundary") = getParam<BoundaryName>("master");

  _problem->addDiracKernel(object_name, object_name + "_" + name(), params);
}

void
ThermalContactAction::addMaterials()
{
  if (getParam<std::string>("type") != "GapHeatTransfer")
    return;

  const std::string object_type = "GapConductance";

  InputParameters params = _factory.getValidParams(object_type);
  params.applyParameters(parameters(), {"variable"});

  params.set<std::vector<VariableName>>("variable") = {getParam<NonlinearVariableName>("variable")};
  params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("slave")};

  if (_quadrature)
  {
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");
  }
  else
  {
    params.set<std::vector<VariableName>>("gap_temp") = {_gap_value_name};
    params.set<std::vector<VariableName>>("gap_distance") = {"penetration"};
  }

  _problem->addMaterial(object_type, name() + "_" + "gap_value", params);

  if (_quadrature)
  {
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("slave");
    params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("master")};
    params.set<std::string>("conductivity_name") =
        getParam<std::string>("conductivity_master_name");

    _problem->addMaterial(object_type, name() + "_" + "gap_value_master", params);
  }
}

void
ThermalContactAction::addSlaveFluxVector()
{
  _problem->getNonlinearSystemBase().addVector("slave_flux", false, GHOSTED);
  _problem->getNonlinearSystemBase().zeroVectorForResidual("slave_flux");

  // It is risky to apply this optimization to contact problems
  // since the problem configuration may be changed during Jacobian
  // evaluation. We therefore turn it off for all contact problems so that
  // PETSc-3.8.4 or higher will have the same behavior as PETSc-3.8.3 or older.
  if (!_problem->isSNESMFReuseBaseSetbyUser())
    _problem->setSNESMFReuseBase(false, false);
}
