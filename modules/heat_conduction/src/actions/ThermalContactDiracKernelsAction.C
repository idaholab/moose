//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalContactDiracKernelsAction.h"
#include "Factory.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<ThermalContactDiracKernelsAction>()
{
  InputParameters params = validParams<Action>();
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
  params.addParam<bool>(
      "quadrature", false, "Whether or not to use quadrature point based gap heat transfer");
  return params;
}

ThermalContactDiracKernelsAction::ThermalContactDiracKernelsAction(const InputParameters & params)
  : Action(params)
{
}

void
ThermalContactDiracKernelsAction::act()
{
  if (!getParam<bool>("quadrature"))
  {
    InputParameters params = _factory.getValidParams("GapHeatPointSourceMaster");
    params.applyParameters(parameters());
    params.set<BoundaryName>("boundary") = getParam<BoundaryName>("master");
    _problem->addDiracKernel(
        "GapHeatPointSourceMaster", "GapHeatPointSourceMaster_" + name(), params);
  }
}
