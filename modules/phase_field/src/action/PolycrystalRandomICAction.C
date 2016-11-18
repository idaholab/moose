/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PolycrystalRandomICAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblemBase.h"
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

const Real PolycrystalRandomICAction::_abs_zero_tol = 1e-12;

template<>
InputParameters validParams<PolycrystalRandomICAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<unsigned int>("op_num", "number of order parameters to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  MooseEnum typ_options("continuous discrete");
  params.addParam<MooseEnum>("random_type", typ_options, "The type of random polycrystal initial condition. Whether one order parameter is chosen to be 1 at each node or if each order parameter continuously varies from 0 to 1");

  return params;
}

PolycrystalRandomICAction::PolycrystalRandomICAction(const InputParameters & params) :
    Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base")),
    _random_type(getParam<MooseEnum>("random_type"))
{}

void
PolycrystalRandomICAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the PolycrystalRandomICAction Object\n";
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
    InputParameters poly_params = _factory.getValidParams("PolycrystalRandomIC");
    poly_params.set<VariableName>("variable") = var_name;
    poly_params.set<unsigned int>("op_num") = _op_num;
    poly_params.set<unsigned int>("op_index") = op;
    poly_params.set<unsigned int>("typ") = _random_type;

    //Add initial condition
    _problem->addInitialCondition("PolycrystalRandomIC", "ICs/PolycrystalICs/PolycrystalRandomIC_" + Moose::stringify(op), poly_params);
  }
}

