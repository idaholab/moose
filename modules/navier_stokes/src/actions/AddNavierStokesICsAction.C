/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AddNavierStokesICsAction.h"
#include "NavierStokesApp.h"
#include "NSInitialCondition.h"

// MOOSE includes
#include "AddVariableAction.h"
#include "FEProblem.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/string_to_enum.h"
#include "libmesh/fe.h"

template<>
InputParameters validParams<AddNavierStokesICsAction>()
{
  InputParameters params = validParams<Action>();

  params.addRequiredParam<Real>("initial_pressure", "The initial pressure, assumed constant everywhere");
  params.addRequiredParam<Real>("initial_temperature", "The initial temperature, assumed constant everywhere");
  params.addRequiredParam<RealVectorValue>("initial_velocity", "The initial velocity, assumed constant everywhere");
  params.addRequiredParam<UserObjectName>("fluid_properties", "The name of the user object for fluid properties");

  return params;
}

AddNavierStokesICsAction::AddNavierStokesICsAction(InputParameters parameters) :
    Action(parameters),
    _initial_pressure(getParam<Real>("initial_pressure")),
    _initial_temperature(getParam<Real>("initial_temperature")),
    _initial_velocity(getParam<RealVectorValue>("initial_velocity")),
    _fp_name(getParam<UserObjectName>("fluid_properties"))
{
}

AddNavierStokesICsAction::~AddNavierStokesICsAction()
{
}

void
AddNavierStokesICsAction::act()
{
  // TODO: this list of names shares code with
  // AddNavierStokesVariablesAction, so they should probably inherit
  // from an intermediate base class.
  unsigned int dim = _mesh->dimension();
  std::vector<NonlinearVariableName> names;
  names.push_back("rho");
  names.push_back("rhou");
  if (dim >= 2)
    names.push_back("rhov");
  if (dim >= 3)
    names.push_back("rhow");
  names.push_back("rhoE");

  names.push_back("vel_x");
  if (dim >= 2)
    names.push_back("vel_y");
  if (dim >= 3)
    names.push_back("vel_z");

  names.push_back("pressure");
  names.push_back("temperature");
  names.push_back("enthalpy");
  names.push_back("Mach");
  names.push_back("internal_energy");
  names.push_back("specific_volume");

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
