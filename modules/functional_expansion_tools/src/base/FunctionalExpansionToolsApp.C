//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "FunctionalExpansionToolsApp.h"
#include "FunctionSeries.h"
#include "FunctionSeriesToAux.h"
#include "FXBoundaryValueUserObject.h"
#include "FXBoundaryFluxUserObject.h"
#include "FXFluxBC.h"
#include "FXValueBC.h"
#include "FXValuePenaltyBC.h"
#include "FXVolumeUserObject.h"
#include "MultiAppFXTransfer.h"

template <>
InputParameters
validParams<FunctionalExpansionToolsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

FunctionalExpansionToolsApp::FunctionalExpansionToolsApp(InputParameters parameters)
  : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  FunctionalExpansionToolsApp::registerObjectDepends(_factory);
  FunctionalExpansionToolsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  FunctionalExpansionToolsApp::associateSyntaxDepends(_syntax, _action_factory);
  FunctionalExpansionToolsApp::associateSyntax(_syntax, _action_factory);
}

FunctionalExpansionToolsApp::~FunctionalExpansionToolsApp() {}

void
FunctionalExpansionToolsApp::registerApps()
{
  registerApp(FunctionalExpansionToolsApp);
}

void
FunctionalExpansionToolsApp::registerObjects(Factory & factory)
{
  registerAuxKernel(FunctionSeriesToAux);

  registerBoundaryCondition(FXValueBC);
  registerBoundaryCondition(FXValuePenaltyBC);
  registerBoundaryCondition(FXFluxBC);

  registerFunction(FunctionSeries);

  registerUserObject(FXBoundaryValueUserObject);
  registerUserObject(FXBoundaryFluxUserObject);
  registerUserObject(FXVolumeUserObject);

  registerTransfer(MultiAppFXTransfer);
}

void
FunctionalExpansionToolsApp::associateSyntax(Syntax & /*syntax*/,
                                             ActionFactory & /*action_factory*/)
{
  /* Uncomment Syntax and ActionFactory parameters and register your new objects here! */
}

void
FunctionalExpansionToolsApp::registerObjectDepends(Factory & /*factory*/)
{
}

void
FunctionalExpansionToolsApp::associateSyntaxDepends(Syntax & /*syntax*/,
                                                    ActionFactory & /*action_factory*/)
{
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
FunctionalExpansionToolsApp__registerApps()
{
  FunctionalExpansionToolsApp::registerApps();
}

extern "C" void
FunctionalExpansionToolsApp__registerObjects(Factory & factory)
{
  FunctionalExpansionToolsApp::registerObjects(factory);
}

extern "C" void
FunctionalExpansionToolsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  FunctionalExpansionToolsApp::associateSyntax(syntax, action_factory);
}
