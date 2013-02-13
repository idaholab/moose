#include "MooseUnitApp.h"
#include "Moose.h"

MooseUnitApp::MooseUnitApp(int argc, char * argv[]) :
    MooseApp(argc, argv)
{
  srand(libMesh::processor_id());
  
  Moose::registerObjects(_factory);
  Moose::associateSyntax(_syntax, _action_factory);
}

MooseUnitApp::~MooseUnitApp()
{
}

