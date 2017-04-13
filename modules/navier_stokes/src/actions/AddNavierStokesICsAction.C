/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "AddNavierStokesICsAction.h"
#include "NavierStokesApp.h"
#include "NSInitialCondition.h"
#include "NSAction.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/string_to_enum.h"
#include "libmesh/fe.h"

template <>
InputParameters
validParams<AddNavierStokesICsAction>()
{
  InputParameters params = validParams<NSAction>();

  params.addClassDescription("This class allows us to have a section of the input file like the "
                             "following which automatically adds initial conditions for all the "
                             "required nonlinear and auxiliary variables.");
  params.addRequiredParam<Real>("initial_pressure",
                                "The initial pressure, assumed constant everywhere");
  params.addRequiredParam<Real>("initial_temperature",
                                "The initial temperature, assumed constant everywhere");
  params.addRequiredParam<RealVectorValue>("initial_velocity",
                                           "The initial velocity, assumed constant everywhere");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  return params;
}

AddNavierStokesICsAction::AddNavierStokesICsAction(InputParameters parameters)
  : NSAction(parameters),
    _initial_pressure(getParam<Real>("initial_pressure")),
    _initial_temperature(getParam<Real>("initial_temperature")),
    _initial_velocity(getParam<RealVectorValue>("initial_velocity")),
    _fp_name(getParam<UserObjectName>("fluid_properties"))
{
}

AddNavierStokesICsAction::~AddNavierStokesICsAction() {}

void
AddNavierStokesICsAction::act()
{
  // Call the base class's act() function to initialize the _vars and _auxs names.
  NSAction::act();

  // Now add the ICs for the nonlinear and auxiliary variables
  addICs(_vars);
  addICs(_auxs);
}

void
AddNavierStokesICsAction::addICs(std::vector<std::string> & names)
{
  for (const auto & name : names)
  {
    InputParameters params = _factory.getValidParams("NSInitialCondition");
    params.set<VariableName>("variable") = name;
    params.set<Real>("initial_pressure") = _initial_pressure;
    params.set<Real>("initial_temperature") = _initial_temperature;
    params.set<RealVectorValue>("initial_velocity") = _initial_velocity;
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    _problem->addInitialCondition("NSInitialCondition", name + std::string("_ic"), params);
  }
}
