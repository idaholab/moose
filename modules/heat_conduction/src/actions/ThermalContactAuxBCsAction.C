//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalContactAuxBCsAction.h"
#include "ThermalContactAuxVarsAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "AddVariableAction.h"

template <>
InputParameters
validParams<ThermalContactAuxBCsAction>()
{
  InputParameters params = validParams<Action>();
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
  return params;
}

ThermalContactAuxBCsAction::ThermalContactAuxBCsAction(const InputParameters & params)
  : Action(params)
{
}

void
ThermalContactAuxBCsAction::act()
{
  bool quadrature = getParam<bool>("quadrature");

  InputParameters params = _factory.getValidParams(getParam<std::string>("gap_aux_type"));
  params.applyParameters(parameters(), {"variable"});
  params.set<AuxVariableName>("variable") = ThermalContactAuxVarsAction::getGapValueName(_pars);

  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_LINEAR};

  params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("slave")};
  params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");
  params.set<VariableName>("paired_variable") = getParam<NonlinearVariableName>("variable");
  _problem->addAuxKernel(getParam<std::string>("gap_aux_type"), "gap_value_" + name(), params);

  if (quadrature)
  {
    params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("master")};
    params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("slave");
    _problem->addAuxKernel(
        getParam<std::string>("gap_aux_type"), "gap_value_master_" + name(), params);
  }

  params = _factory.getValidParams("PenetrationAux");
  params.applyParameters(parameters(), {"variable"});
  std::string penetration_var_name = quadrature ? "qpoint_penetration" : "penetration";
  params.set<AuxVariableName>("variable") = penetration_var_name;

  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_LINEAR};
  ;

  params.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("slave")};
  params.set<BoundaryName>("paired_boundary") = getParam<BoundaryName>("master");

  _problem->addAuxKernel("PenetrationAux", "penetration_" + name(), params);
}
