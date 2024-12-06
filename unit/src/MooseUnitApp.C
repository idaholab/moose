//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseUnitApp.h"
#include "Moose.h"

InputParameters
MooseUnitApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  return params;
}

MooseUnitApp::MooseUnitApp(const InputParameters & parameters) : MooseApp(parameters)
{
  srand(processor_id());
  MooseUnitApp::registerAll(_factory, _action_factory, _syntax);
}

void
MooseUnitApp::registerAll(Factory & f, ActionFactory &, Syntax &)
{
  Registry::registerObjectsTo(f, {"MooseUnitApp"});
}

MooseUnitApp::~MooseUnitApp() {}

InputParameters
OtherMooseUnitApp::validParams()
{
  return MooseApp::validParams();
}

OtherMooseUnitApp::OtherMooseUnitApp(const InputParameters & parameters) : MooseApp(parameters)
{
  OtherMooseUnitApp::registerAll(_factory, _action_factory, _syntax);
}

void
OtherMooseUnitApp::registerAll(Factory & f, ActionFactory &, Syntax &)
{
  Registry::registerObjectsTo(f, {"OtherMooseUnitApp"});
}
