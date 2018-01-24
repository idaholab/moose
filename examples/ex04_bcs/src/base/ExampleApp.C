/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE Includes
#include "ExampleApp.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

// Example 4 Includes
#include "ExampleConvection.h"
#include "ExampleGaussContForcing.h"
#include "CoupledDirichletBC.h"
#include "CoupledNeumannBC.h"

template <>
InputParameters
validParams<ExampleApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ExampleApp::ExampleApp(InputParameters parameters) : MooseApp(parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  ExampleApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ExampleApp::associateSyntax(_syntax, _action_factory);
}

void
ExampleApp::registerObjects(Factory & factory)
{
  registerKernel(ExampleConvection);
  registerKernel(ExampleGaussContForcing);       // Extra forcing term
  registerBoundaryCondition(CoupledDirichletBC); // Register our Boundary Conditions
  registerBoundaryCondition(CoupledNeumannBC);
}

void
ExampleApp::registerApps()
{
  registerApp(ExampleApp);
}

void
ExampleApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
