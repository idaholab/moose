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

#include "SubChannelTestApp.h"
#include "SubChannelApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
SubChannelTestApp::validParams()
{
  InputParameters params = SubChannelApp::validParams();
  return params;
}

SubChannelTestApp::SubChannelTestApp(InputParameters parameters) : MooseApp(parameters)
{
  SubChannelTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

SubChannelTestApp::~SubChannelTestApp() {}

void
SubChannelTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  SubChannelApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"SubChannelTestApp"});
    Registry::registerActionsTo(af, {"SubChannelTestApp"});
  }
}

void
SubChannelTestApp::registerApps()
{
  registerApp(SubChannelTestApp);
  SubChannelApp::registerApps();
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
SubChannelTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SubChannelTestApp::registerAll(f, af, s);
}
extern "C" void
SubChannelTestApp__registerApps()
{
  SubChannelTestApp::registerApps();
}
