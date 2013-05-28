#include "ExampleApp.h"
#include "Example.h"
#include "Moose.h"

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
  Example::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  Example::associateSyntax(_syntax, _action_factory);
}

ExampleApp::~ExampleApp()
{
}

