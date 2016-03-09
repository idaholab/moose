/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "XFEMVolFracAux.h"
#include "XFEMCutPlaneAux.h"
#include "XFEMMarkerAux.h"
#include "XFEMMarkerUserObject.h"
#include "XFEMMaterialTensorMarkerUserObject.h"
#include "XFEMAction.h"
#include "XFEMEqualValueConstraint.h"

template<>
InputParameters validParams<XFEMApp>()
{
  InputParameters params = validParams<MooseApp>();

  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  return params;
}
XFEMApp::XFEMApp(const InputParameters &parameters) :
    MooseApp(parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  XFEMApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  XFEMApp::associateSyntax(_syntax, _action_factory);
}

XFEMApp::~XFEMApp()
{
}

// External entry point for dynamic application loading
extern "C" void XFEMApp__registerApps() { XFEMApp::registerApps(); }
void
XFEMApp::registerApps()
{
  registerApp(XFEMApp);
}

// External entry point for dynamic object registration
extern "C" void XFEMApp__registerObjects(Factory & factory) { XFEMApp::registerObjects(factory); }
void
XFEMApp::registerObjects(Factory & factory)
{
  //AuxKernels
  registerAux(XFEMVolFracAux);
  registerAux(XFEMCutPlaneAux);
  registerAux(XFEMMarkerAux);

  //Constraints
  registerConstraint(XFEMEqualValueConstraint);

  //UserObjects
  registerUserObject(XFEMMarkerUserObject);
  registerUserObject(XFEMMaterialTensorMarkerUserObject);
}

// External entry point for dynamic syntax association
extern "C" void XFEMApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { XFEMApp::associateSyntax(syntax, action_factory); }
void
XFEMApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  registerTask("setup_xfem", false);
  registerAction(XFEMAction, "setup_xfem");
  syntax.addDependency("setup_xfem","setup_adaptivity");
  registerAction(XFEMAction, "add_aux_variable");
  registerAction(XFEMAction, "add_aux_kernel");

  syntax.registerActionSyntax("XFEMAction", "XFEM");
  syntax.registerActionSyntax("AddUserObjectAction", "XFEM/*");

}
