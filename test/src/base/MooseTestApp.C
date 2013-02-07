#include "MooseTestApp.h"
#include "MooseTest.h"
#include "Moose.h"

MooseTestApp::MooseTestApp(int argc, char * argv[]) :
    MooseApp(argc, argv)
{
  srand(libMesh::processor_id());

  Moose::registerObjects(_factory);
  MooseTest::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  MooseTest::associateSyntax(_syntax, _action_factory);
}

MooseTestApp::~MooseTestApp()
{
}

