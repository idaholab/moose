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
#include "MooseSyntax.h"

template <>
InputParameters
validParams<MooseUnitApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

MooseUnitApp::MooseUnitApp(const InputParameters & parameters) : MooseApp(parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  Moose::associateSyntax(_syntax, _action_factory);
}

MooseUnitApp::~MooseUnitApp() {}
