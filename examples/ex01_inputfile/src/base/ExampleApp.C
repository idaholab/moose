#include "ExampleApp.h"
#include "Example.h"
#include "Moose.h"

ExampleApp::ExampleApp(int argc, char * argv[]) :
    MooseApp(argc, argv)
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

