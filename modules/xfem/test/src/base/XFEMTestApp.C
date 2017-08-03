#include "XFEMTestApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<XFEMTestApp>()
{
  InputParameters params = validParams<XFEMApp>();
  return params;
}

XFEMTestApp::XFEMTestApp(InputParameters parameters) : XFEMApp(parameters)
{
  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    XFEMTestApp::registerObjects(_factory);
    XFEMTestApp::associateSyntax(_syntax, _action_factory);
  }
}

XFEMTestApp::~XFEMTestApp() {}

// External entry point for dynamic application loading
extern "C" void
XFEMTestApp__registerApps()
{
  XFEMTestApp::registerApps();
}
void
XFEMTestApp::registerApps()
{
  registerApp(XFEMApp);
  registerApp(XFEMTestApp);
}

// External entry point for dynamic object registration
extern "C" void
XFEMTestApp__registerObjects(Factory & factory)
{
  XFEMTestApp::registerObjects(factory);
}
void
XFEMTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
XFEMTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  XFEMTestApp::associateSyntax(syntax, action_factory);
}
void
XFEMTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
