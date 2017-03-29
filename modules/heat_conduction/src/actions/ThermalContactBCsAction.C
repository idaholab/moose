/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ThermalContactBCsAction.h"
#include "ThermalContactAuxVarsAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "Conversion.h"
#include "AddVariableAction.h"

template <>
InputParameters
validParams<ThermalContactBCsAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>(
      "appended_property_name", "", "Name appended to material properties to make them unique");
  params.addRequiredParam<std::string>(
      "type",
      "A string representing the Moose object that will be used for heat conduction over the gap");
  params.addRequiredParam<NonlinearVariableName>("variable", "The variable for thermal contact");
  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");

  // TODO: these are only used in one Bison test. Deprecate soon!
  params.addParam<std::vector<VariableName>>("disp_x", "The x displacement");
  params.addParam<std::vector<VariableName>>("disp_y", "The y displacement");
  params.addParam<std::vector<VariableName>>("disp_z", "The z displacement");

  params.addParam<std::vector<NonlinearVariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>("order", orders, "The finite element order");

  params.addParam<bool>(
      "warnings", false, "Whether to output warning messages concerning nodes not being found");
  params.addParam<std::vector<AuxVariableName>>(
      "save_in", "The Auxiliary Variable to (optionally) save the boundary flux in");
  params.addParam<bool>(
      "quadrature", false, "Whether or not to use quadrature point based gap heat transfer");

  return params;
}

ThermalContactBCsAction::ThermalContactBCsAction(const InputParameters & params) : Action(params) {}

void
ThermalContactBCsAction::act()
{
  const bool quadrature = getParam<bool>("quadrature");

  InputParameters params = _factory.getValidParams(getParam<std::string>("type"));
  params.applyParameters(parameters());

  if (!quadrature)
  {
    params.set<std::vector<VariableName>>("gap_distance") = {"penetration"};
    params.set<std::vector<VariableName>>("gap_temp") = {
        ThermalContactAuxVarsAction::getGapValueName(_pars)};
  }
  else
  {
    params.set<bool>("quadrature") = true;
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");
    params.set<bool>("use_displaced_mesh") = true;
  }

  params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("slave")};
  _problem->addBoundaryCondition(getParam<std::string>("type"), "gap_bc_" + name(), params);

  if (quadrature)
  {
    // Swap master and slave for this one
    params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("master")};
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("slave");

    _problem->addBoundaryCondition(
        getParam<std::string>("type"), "gap_bc_master_" + name(), params);
  }
}
