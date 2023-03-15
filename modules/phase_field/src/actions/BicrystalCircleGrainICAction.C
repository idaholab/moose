//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BicrystalCircleGrainICAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

registerMooseAction("PhaseFieldApp", BicrystalCircleGrainICAction, "add_ic");

InputParameters
BicrystalCircleGrainICAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Bicrystal with a circular grain and an embedding outer grain");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addRequiredParam<unsigned int>("op_num", "Number of grains, should be 2");
  params.addRequiredParam<Real>("radius", "Void radius");
  params.addRequiredParam<Real>("x", "The x coordinate of the circle grain center");
  params.addRequiredParam<Real>("y", "The y coordinate of the circle grain center");
  params.addParam<Real>("z", 0.0, "The z coordinate of the circle grain center");
  params.addParam<Real>(
      "int_width", 0.0, "The interfacial width of the void surface.  Defaults to sharp interface");
  params.addParam<bool>(
      "3D_sphere", true, "in 3D, whether the smaller grain is a spheres or columnar grain");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "Block restriction for the initial condition");

  return params;
}

BicrystalCircleGrainICAction::BicrystalCircleGrainICAction(const InputParameters & params)
  : Action(params),
    _var_name_base(getParam<std::string>("var_name_base")),
    _op_num(getParam<unsigned int>("op_num")),
    _radius(getParam<Real>("radius")),
    _x(getParam<Real>("x")),
    _y(getParam<Real>("y")),
    _z(getParam<Real>("z")),
    _int_width(getParam<Real>("int_width")),
    _3D_sphere(getParam<bool>("3D_sphere"))
{
  if (_op_num != 2)
    paramError("op_num", "op_num must equal 2 for bicrystal ICs");
}

void
BicrystalCircleGrainICAction::act()
{
  // Loop through the number of order parameters
  for (unsigned int op = 0; op < _op_num; op++)
  {
    // Create variable names
    std::string var_name = _var_name_base + Moose::stringify(op);

    // Set parameters for SmoothCircleIC
    InputParameters poly_params = _factory.getValidParams("SmoothCircleIC");
    poly_params.set<VariableName>("variable") = var_name;
    poly_params.set<Real>("x1") = _x;
    poly_params.set<Real>("y1") = _y;
    poly_params.set<Real>("z1") = _z;
    poly_params.set<Real>("radius") = _radius;
    poly_params.set<Real>("int_width") = _int_width;
    poly_params.set<bool>("3D_spheres") = _3D_sphere;
    if (op == 0)
    {
      // Values for circle grain
      poly_params.set<Real>("invalue") = 1.0;
      poly_params.set<Real>("outvalue") = 0.0;
    }
    else
    {
      // Values for matrix grain
      poly_params.set<Real>("invalue") = 0.0;
      poly_params.set<Real>("outvalue") = 1.0;
    }
    poly_params.applySpecificParameters(_pars, {"block"});

    // Add initial condition
    _problem->addInitialCondition(
        "SmoothCircleIC", "BicrystalCircleGrainIC_" + Moose::stringify(op), poly_params);
  }
}
