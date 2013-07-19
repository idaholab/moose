#include "ExampleApp.h"
#include "Moose.h"
#include "Factory.h"
#include "AppFactory.h"

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
