/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "SubChannelApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"
#include "SubChannelSyntax.h"
#ifdef BISON_ENABLED
#include "BisonApp.h"
#endif

const std::string SubChannelApp::MASS_FLOW_RATE = "mdot";
const std::string SubChannelApp::SURFACE_AREA = "S";
const std::string SubChannelApp::SUM_CROSSFLOW = "SumWij";
const std::string SubChannelApp::PRESSURE = "P";
const std::string SubChannelApp::PRESSURE_DROP = "DP";
const std::string SubChannelApp::ENTHALPY = "h";
const std::string SubChannelApp::TEMPERATURE = "T";
const std::string SubChannelApp::PIN_TEMPERATURE = "Tpin";
const std::string SubChannelApp::DENSITY = "rho";
const std::string SubChannelApp::VISCOSITY = "mu";
const std::string SubChannelApp::WETTED_PERIMETER = "w_perim";
const std::string SubChannelApp::LINEAR_HEAT_RATE = "q_prime";
const std::string SubChannelApp::DUCT_LINEAR_HEAT_RATE = "q_prime_duct";
const std::string SubChannelApp::DUCT_TEMPERATURE = "Tduct";

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
  ModulesApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"SubChannelApp"});
  Registry::registerActionsTo(af, {"SubChannelApp"});

  /* register custom execute flags, action syntax, etc. here */
  SubChannel::associateSyntax(s, af);

#ifdef BISON_ENABLED
  BisonApp::registerAll(f, af, s);
#endif
}

void
SubChannelApp::registerApps()
{
  registerApp(SubChannelApp);
#ifdef BISON_ENABLED
  BisonApp::registerApps();
#endif
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
