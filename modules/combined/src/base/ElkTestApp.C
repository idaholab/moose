#include "ElkTestApp.h"
#include "Elk.h"

ElkTestApp::ElkTestApp(int argc, char * argv[]) :
    MooseApp(argc, argv)
{
  Moose::registerObjects(_factory);
  Elk::registerObjects(_factory);
  
  Moose::associateSyntax(_syntax, _action_factory);
  Elk::associateSyntax(_syntax, _action_factory);
}

ElkTestApp::~ElkTestApp()
{
}

