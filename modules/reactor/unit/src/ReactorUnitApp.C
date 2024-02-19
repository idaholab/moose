//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactorUnitApp.h"

InputParameters
ReactorUnitApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  return params;
}

ReactorUnitApp::ReactorUnitApp(const InputParameters & parameters) : ReactorApp(parameters)
{
  srand(processor_id());
  ReactorUnitApp::registerAll(_factory, _action_factory, _syntax);
}

void
ReactorUnitApp::registerAll(Factory & f, ActionFactory &, Syntax &)
{
  Registry::registerObjectsTo(f, {"ReactorUnitApp"});
}

ReactorUnitApp::~ReactorUnitApp() {}
