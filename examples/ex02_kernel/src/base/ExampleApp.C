#include "ExampleApp.h"
#include "Moose.h"
#include "Factory.h"
#include "AppFactory.h"

// Example 2 Includes
#include "Convection.h"           // <- New include for our custom kernel

template<>
InputParameters validParams<ExampleApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ExampleApp::ExampleApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(libMesh::processor_id());

  Moose::registerObjects(_factory);
  ExampleApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ExampleApp::associateSyntax(_syntax, _action_factory);
}

ExampleApp::~ExampleApp()
{
}

void
ExampleApp::registerObjects(Factory & factory)
{
  // Register any custom objects you have built on the MOOSE Framework
  registerKernel(Convection);  // <- registration
}

void
ExampleApp::registerApps()
{
  registerApp(ExampleApp);
}

void
ExampleApp::associateSyntax(Syntax& syntax, ActionFactory & action_factory)
{
}
