#include "PolycrystalVoronoiVoidICAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Conversion.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"

const Real PolycrystalVoronoiVoidICAction::_abs_zero_tol = 1e-12;

template<>
InputParameters validParams<PolycrystalVoronoiVoidICAction>()
{
  InputParameters params = validParams<Action>();
  //MooseEnum structure_options("grains voids");
  //params.addRequiredParam<MooseEnum>("structure_type", structure_options, "Which structure type is being initialized, grains or voids");
  params.addRequiredParam<unsigned int>("op_num", "number of order parameters to create");
  params.addRequiredParam<unsigned int>("grain_num", "number of grains to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<unsigned int>("rand_seed", 12444, "The random seed");

  params.addParam<bool>("columnar_3D", false, "3D grain structure will be columnar in the z-direction?");

  params.addRequiredParam<Real>("invalue", "The variable value inside the circle");
  params.addRequiredParam<Real>("outvalue", "The variable value outside the circle");
  params.addParam<Real>("int_width", 0.0, "The interfacial width of the void surface.  Defaults to sharp interface");
  params.addParam<bool>("3D_spheres", true, "in 3D, whether the objects are spheres or columns");
  params.addRequiredParam<unsigned int>("numbub", "The number of bubbles to be placed on GB");
  params.addRequiredParam<Real>("bubspac", "minimum spacing of bubbles, measured from center to center");
  params.addParam<unsigned int>("numtries", 1000, "The number of tries");
  params.addRequiredParam<Real>("radius", "Mean radius value for the circels");
  params.addParam<Real>("radius_variation", 0.0, "Plus or minus fraction of random variation in the bubble radius for uniform, standard deviation for normal");
  MooseEnum rand_options("uniform normal none","none");
  params.addParam<MooseEnum>("radius_variation_type", rand_options, "Type of distribution that random circle radii will follow");

  return params;
}

PolycrystalVoronoiVoidICAction::PolycrystalVoronoiVoidICAction(const InputParameters & params) :
    Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _grain_num(getParam<unsigned int>("grain_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{}

void
PolycrystalVoronoiVoidICAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the PolycrystalVoronoiVoidICAction Object\n";
#endif

  // Loop through the number of order parameters
  for (unsigned int op = 0; op < _op_num; op++)
  {
    //Create variable names
    std::string var_name = _var_name_base;
    std::stringstream out;
    out << op;
    var_name.append(out.str());

    //Set parameters for BoundingBoxIC
    InputParameters poly_params = _factory.getValidParams("PolycrystalVoronoiVoidIC");
    poly_params.set<VariableName>("variable") = var_name;
    poly_params.set<MooseEnum>("structure_type") = "grains";
    poly_params.set<unsigned int>("op_num") = _op_num;
    poly_params.set<unsigned int>("grain_num") = _grain_num;
    poly_params.set<unsigned int>("op_index") = op;
    poly_params.set<unsigned int>("rand_seed") = getParam<unsigned int>("rand_seed");
    poly_params.set<bool>("columnar_3D") = getParam<bool>("columnar_3D");
    poly_params.set<Real>("invalue") = getParam<Real>("invalue");
    poly_params.set<Real>("outvalue") = getParam<Real>("outvalue");
    poly_params.set<Real>("int_width") = getParam<Real>("int_width");
    poly_params.set<bool>("3D_spheres") = getParam<bool>("3D_spheres");
    poly_params.set<unsigned int>("numbub") = getParam<unsigned int>("numbub");
    poly_params.set<Real>("bubspac") = getParam<Real>("bubspac");
    poly_params.set<Real>("radius") = getParam<Real>("radius");
    poly_params.set<Real>("radius_variation") = getParam<Real>("radius_variation");
    poly_params.set<MooseEnum>("radius_variation_type") = getParam<MooseEnum>("radius_variation_type");

    //Add initial condition
    _problem->addInitialCondition("PolycrystalVoronoiVoidIC", name() + "_" + Moose::stringify(op), poly_params);
  }
}
