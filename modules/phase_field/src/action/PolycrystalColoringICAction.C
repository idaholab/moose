/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PolycrystalColoringICAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "PolycrystalICTools.h"

template <>
InputParameters
validParams<PolycrystalColoringICAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Random Voronoi tesselation polycrystal action");
  params.addRequiredParam<unsigned int>("op_num", "number of order parameters to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addRequiredParam<UserObjectName>("polycrystal_ic_uo", "Optional: TODO");

  return params;
}

PolycrystalColoringICAction::PolycrystalColoringICAction(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{
}

void
PolycrystalColoringICAction::act()
{
  // Loop through the number of order parameters
  for (unsigned int op = 0; op < _op_num; op++)
  {
    // Set parameters for BoundingBoxIC
    InputParameters poly_params = _factory.getValidParams("PolycrystalColoringIC");
    poly_params.set<VariableName>("variable") = _var_name_base + Moose::stringify(op);
    poly_params.set<unsigned int>("op_index") = op;
    poly_params.set<UserObjectName>("polycrystal_ic_uo") =
        getParam<UserObjectName>("polycrystal_ic_uo");

    // Add initial condition
    _problem->addInitialCondition(
        "PolycrystalColoringIC", "PolycrystalColoringIC_" + Moose::stringify(op), poly_params);
  }
}
