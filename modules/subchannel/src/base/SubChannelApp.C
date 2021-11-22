#include "SubChannelApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
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
const std::string SubChannelApp::DENSITY = "rho";
const std::string SubChannelApp::VISCOSITY = "mu";
const std::string SubChannelApp::WETTED_PERIMETER = "w_perim";
const std::string SubChannelApp::LINEAR_HEAT_RATE = "q_prime";

InputParameters
SubChannelApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  return params;
}

SubChannelApp::SubChannelApp(InputParameters parameters) : MooseApp(parameters)
{
  SubChannelApp::registerAll(_factory, _action_factory, _syntax);
}

SubChannelApp::~SubChannelApp() {}

void
SubChannelApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"SubChannelApp"});
  Registry::registerActionsTo(af, {"SubChannelApp"});

  /* register custom execute flags, action syntax, etc. here */
  SubChannel::associateSyntax(s, af);
}

void
SubChannelApp::registerApps()
{
  registerApp(SubChannelApp);
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
