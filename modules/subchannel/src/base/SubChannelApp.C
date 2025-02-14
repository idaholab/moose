//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubChannelApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "FluidPropertiesApp.h"
#include "HeatTransferApp.h"
#include "ReactorApp.h"
#include "MooseSyntax.h"
#include "SubChannelSyntax.h"

const std::string SubChannelApp::MASS_FLOW_RATE = "mdot";
const std::string SubChannelApp::SURFACE_AREA = "S";
const std::string SubChannelApp::SUM_CROSSFLOW = "SumWij";
const std::string SubChannelApp::PRESSURE = "P";
const std::string SubChannelApp::PRESSURE_DROP = "DP";
const std::string SubChannelApp::ENTHALPY = "h";
const std::string SubChannelApp::TEMPERATURE = "T";
const std::string SubChannelApp::PIN_TEMPERATURE = "Tpin";
const std::string SubChannelApp::PIN_DIAMETER = "Dpin";
const std::string SubChannelApp::DENSITY = "rho";
const std::string SubChannelApp::VISCOSITY = "mu";
const std::string SubChannelApp::WETTED_PERIMETER = "w_perim";
const std::string SubChannelApp::LINEAR_HEAT_RATE = "q_prime";
const std::string SubChannelApp::DUCT_LINEAR_HEAT_RATE = "q_prime_duct";
const std::string SubChannelApp::DUCT_TEMPERATURE = "Tduct";
const std::string SubChannelApp::DISPLACEMENT = "displacement";

InputParameters
SubChannelApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  return params;
}

SubChannelApp::SubChannelApp(const InputParameters & parameters) : MooseApp(parameters)
{
  SubChannelApp::registerAll(_factory, _action_factory, _syntax);
}

void
SubChannelApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"SubChannelApp"});
  Registry::registerActionsTo(af, {"SubChannelApp"});

  /* register custom execute flags, action syntax, etc. here */
  SubChannel::associateSyntax(s, af);

  FluidPropertiesApp::registerAll(f, af, s);
  HeatTransferApp::registerAll(f, af, s);
  ReactorApp::registerAll(f, af, s);
}

void
SubChannelApp::registerApps()
{
  registerApp(SubChannelApp);

  FluidPropertiesApp::registerApps();
  HeatTransferApp::registerApps();
  ReactorApp::registerApps();
}

void
SubChannelApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"SubChannelApp"});
}

void
SubChannelApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"SubChannelApp"});
}

void
SubChannelApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
SubChannelApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SubChannelApp::registerAll(f, af, s);
}
extern "C" void
SubChannelApp__registerApps()
{
  SubChannelApp::registerApps();
}
