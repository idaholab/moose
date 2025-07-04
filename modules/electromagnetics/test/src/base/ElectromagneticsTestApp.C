//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElectromagneticsTestApp.h"
#include "ElectromagneticsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ElectromagneticsTestApp::validParams()
{
  InputParameters params = ElectromagneticsApp::validParams();
  return params;
}

registerKnownLabel("ElectromagneticsTestApp");

ElectromagneticsTestApp::ElectromagneticsTestApp(const InputParameters & parameters)
  : MooseApp(parameters)
{
  ElectromagneticsTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

ElectromagneticsTestApp::~ElectromagneticsTestApp() {}

void
ElectromagneticsTestApp::registerAll(Factory & f,
                                     ActionFactory & af,
                                     Syntax & s,
                                     bool use_test_objs)
{
  ElectromagneticsApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"ElectromagneticsTestApp"});
    Registry::registerActionsTo(af, {"ElectromagneticsTestApp"});
  }
}

void
ElectromagneticsTestApp::registerApps()
{
  ElectromagneticsApp::registerApps();
  registerApp(ElectromagneticsTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
ElectromagneticsTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ElectromagneticsTestApp::registerAll(f, af, s);
}
extern "C" void
ElectromagneticsTestApp__registerApps()
{
  ElectromagneticsTestApp::registerApps();
}
