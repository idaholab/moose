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

// Counter for naming materials
static unsigned int materials_counter = 0;

// Counter for naming dirac kernels
static unsigned int dirac_counter = 0;

// Counter for naming BCs
static unsigned int bcs_counter = 0;

// Counter for naming aux kernels
static unsigned int aux_kenels_counter = 0;

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
  params.addParam<std::vector<BoundaryName>>(
      "primary", "The list of boundary IDs referring to primary sidesets");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "secondary", "The list of boundary IDs referring to secondary sidesets");
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
  params.addParam<VariableName>("secondary_gap_offset",
                                "Offset to gap distance from secondary side");
  params.addParam<VariableName>("mapped_primary_gap_offset",
                                "Offset to gap distance mapped from primary side");
  params.addParam<bool>(
      "check_boundary_restricted",
      true,
      "Whether to check for multiple element sides on the boundary for the boundary restricted, "
      "element aux variable set up for thermal contact enforcement. Setting this to false will "
      "allow contribution to a single element's elemental value(s) from multiple boundary sides "
      "on the same element (example: when the restricted boundary exists on two or more sides "
      "of an element, such as at a corner of a mesh");

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
    _gap_conductivity_name("paired_k_" + getParam<NonlinearVariableName>("variable")),
    _boundary_pairs(getParam<BoundaryName, BoundaryName>("primary", "secondary"))
{
  if (!params.get<bool>("check_boundary_restricted"))
  {
    if (_quadrature)
      paramInfo(
          "check_boundary_restricted",
          "This parameter is set to 'false'. Although thermal contact ",
          "will be correctly enforced, the contact-related output may have issues ",
          "in cases where where more than one face of an element belongs to a contact surface ",
          "because the values from only one of the faces will be reported.");
    else
      paramError("check_boundary_restricted",
                 "This parameter cannot be 'false' when 'quadrature=false'");
  }
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
    addSecondaryFluxVector();
}

void
ThermalContactAction::addAuxKernels()
{
  for (const auto & contact_pair : _boundary_pairs)
  {
    // Add gap aux kernel
    {
      InputParameters params = _factory.getValidParams(getParam<std::string>("gap_aux_type"));

      params.applySpecificParameters(parameters(),
                                     {"tangential_tolerance",
                                      "normal_smoothing_distance",
                                      "normal_smoothing_method",
                                      "order",
                                      "warnings",
                                      "check_boundary_restricted"});
      params.set<AuxVariableName>("variable") = _gap_value_name;
      params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_LINEAR};

      params.set<std::vector<BoundaryName>>("boundary") = {contact_pair.second};
      params.set<BoundaryName>("paired_boundary") = contact_pair.first;
      params.set<VariableName>("paired_variable") = getParam<NonlinearVariableName>("variable");

      _problem->addAuxKernel(getParam<std::string>("gap_aux_type"),
                             "gap_value_" + name() + "_" + Moose::stringify(aux_kenels_counter),
                             params);

      if (_quadrature)
      {
        params.set<std::vector<BoundaryName>>("boundary") = {contact_pair.first};
        params.set<BoundaryName>("paired_boundary") = contact_pair.second;

        _problem->addAuxKernel(getParam<std::string>("gap_aux_type"),
                               "gap_value_primary_" + name() + "_" +
                                   Moose::stringify(aux_kenels_counter),
                               params);
      }
    }

    // Add penetration aux kernel
    {
      InputParameters params = _factory.getValidParams("PenetrationAux");

      params.applySpecificParameters(parameters(),
                                     {"tangential_tolerance",
                                      "normal_smoothing_distance",
                                      "normal_smoothing_method",
                                      "order",
                                      "check_boundary_restricted"});
      params.set<AuxVariableName>("variable") = _penetration_var_name;
      if (isParamValid("secondary_gap_offset"))
        params.set<std::vector<VariableName>>("secondary_gap_offset") = {
            getParam<VariableName>("secondary_gap_offset")};
      if (isParamValid("mapped_primary_gap_offset"))
        params.set<std::vector<VariableName>>("mapped_primary_gap_offset") = {
            getParam<VariableName>("mapped_primary_gap_offset")};
      params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_LINEAR};
      params.set<std::vector<BoundaryName>>("boundary") = {contact_pair.second};
      params.set<BoundaryName>("paired_boundary") = contact_pair.first;

      _problem->addAuxKernel("PenetrationAux",
                             "penetration_" + name() + "_" + Moose::stringify(aux_kenels_counter++),
                             params);
    }
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
      AddVariableAction::variableType(FEType(order, Utility::string_to_enum<FEFamily>(family)));
  auto var_params = _factory.getValidParams(var_type);
  var_params.set<MooseEnum>("order") = order;
  var_params.set<MooseEnum>("family") = family;

  _problem->addAuxVariable(var_type, _penetration_var_name, var_params);
  _problem->addAuxVariable(var_type, _gap_value_name, var_params);
}

void
ThermalContactAction::addBCs()
{
  for (const auto & contact_pair : _boundary_pairs)
  {
    const std::string object_name = getParam<std::string>("type");
    InputParameters params = _factory.getValidParams(object_name);
    params.applyParameters(parameters());

    if (_quadrature)
    {
      params.set<BoundaryName>("paired_boundary") = contact_pair.first;
      params.set<bool>("use_displaced_mesh") = true;
    }
    else
    {
      params.set<std::vector<VariableName>>("gap_distance") = {"penetration"};
      params.set<std::vector<VariableName>>("gap_temp") = {_gap_value_name};
    }

    params.set<std::vector<BoundaryName>>("boundary") = {contact_pair.second};

    _problem->addBoundaryCondition(
        object_name, "gap_bc_" + name() + "_" + Moose::stringify(bcs_counter), params);

    if (_quadrature)
    {
      // Swap primary and secondary for this one
      params.set<std::vector<BoundaryName>>("boundary") = {contact_pair.first};
      params.set<BoundaryName>("paired_boundary") = contact_pair.second;

      _problem->addBoundaryCondition(
          object_name, "gap_bc_primary_" + name() + "_" + Moose::stringify(bcs_counter), params);
    }
    bcs_counter++;
  }
}

void
ThermalContactAction::addDiracKernels()
{
  if (_quadrature)
    return;

  for (const auto & contact_pair : _boundary_pairs)
  {
    const std::string object_name = "GapHeatPointSourceMaster";
    InputParameters params = _factory.getValidParams(object_name);
    params.applySpecificParameters(parameters(),
                                   {"tangential_tolerance",
                                    "normal_smoothing_distance",
                                    "normal_smoothing_method",
                                    "order",
                                    "variable"});
    params.set<BoundaryName>("boundary") = contact_pair.first;
    params.set<BoundaryName>("secondary") = contact_pair.second;

    _problem->addDiracKernel(
        object_name, object_name + "_" + name() + "_" + Moose::stringify(dirac_counter++), params);
  }
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

    for (const auto & contact_pair : _boundary_pairs)
    {
      const std::string object_type = "GapConductanceConstant";
      InputParameters params = _factory.getValidParams(object_type);
      params.applyParameters(parameters());
      params.set<std::vector<BoundaryName>>("boundary") = {contact_pair.second};
      _problem->addMaterial(object_type,
                            name() + "_" + "gap_value" + "_" + Moose::stringify(materials_counter),
                            params);

      if (_quadrature)
      {
        params.set<std::vector<BoundaryName>>("boundary") = {contact_pair.first};
        _problem->addMaterial(object_type,
                              name() + "_" + "gap_value_primary" + "_" +
                                  Moose::stringify(materials_counter),
                              params);
      }
      materials_counter++;
    }
  }
  else
  {
    const std::string object_type = "GapConductance";

    for (const auto & contact_pair : _boundary_pairs)
    {
      InputParameters params = _factory.getValidParams(object_type);
      params.applyParameters(parameters(), {"variable"});

      params.set<std::vector<VariableName>>("variable") = {
          getParam<NonlinearVariableName>("variable")};
      params.set<std::vector<BoundaryName>>("boundary") = {contact_pair.second};

      if (_quadrature)
        params.set<BoundaryName>("paired_boundary") = contact_pair.first;
      else
      {
        params.set<std::vector<VariableName>>("gap_temp") = {_gap_value_name};
        params.set<std::vector<VariableName>>("gap_distance") = {"penetration"};
      }

      _problem->addMaterial(object_type,
                            name() + "_" + "gap_value" + "_" + Moose::stringify(materials_counter),
                            params);

      if (_quadrature)
      {
        params.set<BoundaryName>("paired_boundary") = contact_pair.second;
        params.set<std::vector<BoundaryName>>("boundary") = {contact_pair.first};

        _problem->addMaterial(object_type,
                              name() + "_" + "gap_value_primary" + "_" +
                                  Moose::stringify(materials_counter),
                              params);
      }
      materials_counter++;
    }
  }
}

void
ThermalContactAction::addSecondaryFluxVector()
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
