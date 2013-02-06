#include "ElkTestApp.h"
#include "Elk.h"

ElkTestApp::ElkTestApp(int argc, char * argv[]) :
    MooseApp(argc, argv)
{
  init();
  Elk::registerObjects(_factory);
  Elk::associateSyntax(_syntax, _action_factory);
}

ElkTestApp::~ElkTestApp()
{
}

