/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<KKSAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("c_name_base", "c", "base name of the concentration variables");
  params.addParam<std::string>("eta_name", "eta", "name of the order parameter");
  params.addRequiredParam<std::vector<std::string>>("phase_names", "short names for the phases");
  params.addRequiredParam<std::vector<std::string>>("c_names",
                                                    "short names for the concentrations");
  return params;
}

KKSAction::KKSAction(const InputParameters & params) : Action(params)
// _z2(getParam<Real>("z2"))
{
}

void
KKSAction::act()
{
  /*
    InputParameters poly_params = _factory.getValidParams("BoundingBoxIC");
    poly_params.set<VariableName>("variable") = var_name;
    poly_params.set<Real>("x1") = _x1;
    poly_params.set<Real>("y1") = _y1;
    poly_params.set<Real>("z1") = _z1;
    poly_params.set<Real>("x2") = _x2;
    poly_params.set<Real>("y2") = _y2;
    poly_params.set<Real>("z2") = _z2;

    // Add initial condition
    _problem->addInitialCondition("BoundingBoxIC", "InitialCondition", poly_params);
  */
}
