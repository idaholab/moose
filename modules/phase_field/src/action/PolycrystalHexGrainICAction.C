#include "PolycrystalHexGrainICAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblem.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"

const Real PolycrystalHexGrainICAction::_abs_zero_tol = 1e-12;

template<>
InputParameters validParams<PolycrystalHexGrainICAction>()
{
  InputParameters params = validParams<Action>();

  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addRequiredParam<unsigned int>("crys_num", "Number of order parameters");
  params.addRequiredParam<unsigned int>("grain_num", "Number of grains, must be a square (4, 9, 16, etc)");
  params.addParam<unsigned int>("rand_seed", 12444, "The random seed");
  params.addParam<Real>("perturbation_percent", 0.0, "The percent to randomly perturbate centers of grains relative to the size of the grain");

  params.addParam<Real>("x_offset", 0.5, "Specifies offset of hexagon grid in x-direction");

  return params;
}

PolycrystalHexGrainICAction::PolycrystalHexGrainICAction(const std::string & name, InputParameters params) :
    Action(name, params),
    _var_name_base(getParam<std::string>("var_name_base")),
    _crys_num(getParam<unsigned int>("crys_num")),
    _grain_num(getParam<unsigned int>("grain_num")),
    _x_offset(getParam<Real>("x_offset")),
    _perturbation_percent(getParam<Real>("perturbation_percent"))
{}

void
PolycrystalHexGrainICAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the PolycrystalHexGrainICAction Object\n";
#endif

  // Loop through the number of order parameters
  for (unsigned int crys = 0; crys < _crys_num; crys++)
  {
    //Create variable names
    std::string var_name = _var_name_base;
    std::stringstream out;
    out << crys;
    var_name.append(out.str());

    //Set parameters for BoundingBoxIC
    InputParameters poly_params = _factory.getValidParams("HexPolycrystalIC");
    poly_params.set<VariableName>("variable") = var_name;
    poly_params.set<Real>("x_offset") = _x_offset;
    poly_params.set<unsigned int>("crys_num") = _crys_num;
    poly_params.set<unsigned int>("grain_num") = _grain_num;
    poly_params.set<unsigned int>("crys_index") = crys;
    poly_params.set<unsigned int>("rand_seed") = getParam<unsigned int>("rand_seed");
    poly_params.set<Real>("perturbation_percent") = _perturbation_percent;

    //Add initial condition
    _problem->addInitialCondition("HexPolycrystalIC", "InitialCondition", poly_params);
  }
}
