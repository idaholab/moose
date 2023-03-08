//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesUnitApp.h"

InputParameters
NavierStokesUnitApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  return params;
}

NavierStokesUnitApp::NavierStokesUnitApp(const InputParameters & parameters)
  : NavierStokesApp(parameters)
{
  srand(processor_id());
  NavierStokesUnitApp::registerAll(_factory, _action_factory, _syntax);
}

void
NavierStokesUnitApp::registerAll(Factory & f, ActionFactory &, Syntax &)
{
  Registry::registerObjectsTo(f, {"NavierStokesUnitApp"});
}

NavierStokesUnitApp::~NavierStokesUnitApp() {}
