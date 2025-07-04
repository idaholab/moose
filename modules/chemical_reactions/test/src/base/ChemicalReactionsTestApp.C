//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChemicalReactionsTestApp.h"
#include "ChemicalReactionsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ChemicalReactionsTestApp::validParams()
{
  InputParameters params = ChemicalReactionsApp::validParams();
  return params;
}

registerKnownLabel("ChemicalReactionsTestApp");

ChemicalReactionsTestApp::ChemicalReactionsTestApp(const InputParameters & parameters)
  : ChemicalReactionsApp(parameters)
{
  ChemicalReactionsTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

ChemicalReactionsTestApp::~ChemicalReactionsTestApp() {}

void
ChemicalReactionsTestApp::registerAll(Factory & f,
                                      ActionFactory & af,
                                      Syntax & /*s*/,
                                      bool use_test_objects)
{
  if (use_test_objects)
  {
    Registry::registerObjectsTo(f, {"ChemicalReactionsTestApp"});
    Registry::registerActionsTo(af, {"ChemicalReactionsTestApp"});
  }
}

void
ChemicalReactionsTestApp::registerApps()
{
  ChemicalReactionsApp::registerApps();
  registerApp(ChemicalReactionsTestApp);
}

extern "C" void
ChemicalReactionsTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ChemicalReactionsTestApp::registerAll(f, af, s);
}

// External entry point for dynamic application loading
extern "C" void
ChemicalReactionsTestApp__registerApps()
{
  ChemicalReactionsTestApp::registerApps();
}
