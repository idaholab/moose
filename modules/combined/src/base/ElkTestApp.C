#include "ElkTestApp.h"
#include "Elk.h"

ElkTestApp::ElkTestApp(int argc, char * argv[]) :
    MooseApp(argc, argv)
{
  init();
  Elk::registerObjects();
  Elk::associateSyntax(_syntax);
}

ElkTestApp::~ElkTestApp()
{
}

