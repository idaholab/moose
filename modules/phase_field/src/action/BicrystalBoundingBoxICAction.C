/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "BicrystalBoundingBoxICAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

template <>
InputParameters
validParams<BicrystalBoundingBoxICAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Bicrystal using a bounding box");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addRequiredParam<unsigned int>("op_num", "Number of grains, should be 2");
  params.addRequiredParam<Real>("x1", "The x coordinate of the lower left-hand corner of the box");
  params.addRequiredParam<Real>("y1", "The y coordinate of the lower left-hand corner of the box");
  params.addParam<Real>("z1", 0.0, "The z coordinate of the lower left-hand corner of the box");
  params.addRequiredParam<Real>("x2", "The x coordinate of the upper right-hand corner of the box");
  params.addRequiredParam<Real>("y2", "The y coordinate of the upper right-hand corner of the box");
  params.addParam<Real>("z2", 0.0, "The z coordinate of the upper right-hand corner of the box");
  return params;
}

BicrystalBoundingBoxICAction::BicrystalBoundingBoxICAction(const InputParameters & params)
  : Action(params),
    _var_name_base(getParam<std::string>("var_name_base")),
    _op_num(getParam<unsigned int>("op_num"))
{
  if (_op_num != 2)
    mooseError("op_num must equal 2 for bicrystal ICs");
}

void
BicrystalBoundingBoxICAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the BicrystalBoundingBoxICAction Object\n";
#endif

  // Loop through the number of order parameters
  for (unsigned int op = 0; op < _op_num; ++op)
  {
    // Create variable names
    const std::string var_name = _var_name_base + Moose::stringify(op);

    // Set parameters for BoundingBoxIC
    InputParameters poly_params = _factory.getValidParams("BoundingBoxIC");
    poly_params.applyParameters(parameters());
    poly_params.set<VariableName>("variable") = var_name;
    if (op == 0)
    {
      // Values for bounding box
      poly_params.set<Real>("inside") = 1.0;
      poly_params.set<Real>("outside") = 0.0;
    }
    else
    {
      // Values for matrix grain
      poly_params.set<Real>("inside") = 0.0;
      poly_params.set<Real>("outside") = 1.0;
    }

    // Add initial condition
    _problem->addInitialCondition(
        "BoundingBoxIC", "BicrystalBoundingBoxIC_" + Moose::stringify(op), poly_params);
  }
}
