/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PolycrystalVoronoiICAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

template <>
InputParameters
validParams<PolycrystalVoronoiICAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Random Voronoi tesselation polycrystal action");
  params.addRequiredParam<unsigned int>("op_num", "number of order parameters to create");
  params.addRequiredParam<unsigned int>(
      "grain_num", "number of grains to create, if it is going to greater than op_num");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<unsigned int>("rand_seed", 12444, "The random seed");
  params.addParam<bool>(
      "columnar_3D", false, "3D microstructure will be columnar in the z-direction?");
  params.addParam<bool>("advanced_op_assignment",
                        false,
                        "Enable advanced grain to op assignment (avoid invalid graph coloring)");
  return params;
}

PolycrystalVoronoiICAction::PolycrystalVoronoiICAction(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _grain_num(getParam<unsigned int>("grain_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{
}

void
PolycrystalVoronoiICAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the PolycrystalVoronoiICAction Object\n";
#endif

  // Loop through the number of order parameters
  for (unsigned int op = 0; op < _op_num; op++)
  {
    // Set parameters for BoundingBoxIC
    InputParameters poly_params = _factory.getValidParams("PolycrystalReducedIC");
    poly_params.set<VariableName>("variable") = _var_name_base + Moose::stringify(op);
    poly_params.set<unsigned int>("op_num") = _op_num;
    poly_params.set<unsigned int>("grain_num") = _grain_num;
    poly_params.set<unsigned int>("op_index") = op;
    poly_params.set<unsigned int>("rand_seed") = getParam<unsigned int>("rand_seed");
    poly_params.set<bool>("columnar_3D") = getParam<bool>("columnar_3D");
    poly_params.set<bool>("advanced_op_assignment") = getParam<bool>("advanced_op_assignment");

    // Add initial condition
    _problem->addInitialCondition(
        "PolycrystalReducedIC", "PolycrystalVoronoiIC_" + Moose::stringify(op), poly_params);
  }
}
