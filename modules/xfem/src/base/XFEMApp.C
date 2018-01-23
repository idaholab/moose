/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMApp.h"
#include "XFEMAppTypes.h"
#include "SolidMechanicsApp.h"
#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "XFEMVolFracAux.h"
#include "XFEMCutPlaneAux.h"
#include "XFEMMarkerAux.h"
#include "XFEMMaterialTensorMarkerUserObject.h"
#include "XFEMRankTwoTensorMarkerUserObject.h"
#include "XFEMAction.h"
#include "XFEMSingleVariableConstraint.h"
#include "XFEMPressure.h"
#include "CrackTipEnrichmentStressDivergenceTensors.h"
#include "CrackTipEnrichmentCutOffBC.h"
#include "ComputeCrackTipEnrichmentSmallStrain.h"

#include "GeometricCutUserObject.h"
#include "LineSegmentCutUserObject.h"
#include "LineSegmentCutSetUserObject.h"
#include "CircleCutUserObject.h"
#include "EllipseCutUserObject.h"
#include "RectangleCutUserObject.h"

template <>
InputParameters
validParams<XFEMApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}
XFEMApp::XFEMApp(const InputParameters & parameters) : MooseApp(parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  XFEMApp::registerObjectDepends(_factory);
  XFEMApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  XFEMApp::associateSyntaxDepends(_syntax, _action_factory);
  XFEMApp::associateSyntax(_syntax, _action_factory);
}

XFEMApp::~XFEMApp() {}

// External entry point for dynamic application loading
extern "C" void
XFEMApp__registerApps()
{
  XFEMApp::registerApps();
}
void
XFEMApp::registerApps()
{
  registerApp(XFEMApp);
}

void
XFEMApp::registerObjectDepends(Factory & factory)
{
  SolidMechanicsApp::registerObjects(factory);
  TensorMechanicsApp::registerObjects(factory);
}

// External entry point for dynamic object registration
extern "C" void
XFEMApp__registerObjects(Factory & factory)
{
  XFEMApp::registerObjects(factory);
}
void
XFEMApp::registerObjects(Factory & factory)
{
  // AuxKernels
  registerAux(XFEMVolFracAux);
  registerAux(XFEMCutPlaneAux);
  registerAux(XFEMMarkerAux);

  // Constraints
  registerConstraint(XFEMSingleVariableConstraint);

  // UserObjects
  registerUserObject(XFEMMaterialTensorMarkerUserObject);
  registerUserObject(XFEMRankTwoTensorMarkerUserObject);

  // Geometric Cut User Objects
  registerUserObject(LineSegmentCutUserObject);
  registerUserObject(LineSegmentCutSetUserObject);
  registerUserObject(CircleCutUserObject);
  registerUserObject(EllipseCutUserObject);
  registerUserObject(RectangleCutUserObject);

  // DiracKernels
  registerDiracKernel(XFEMPressure);

  // Kernels
  registerKernel(CrackTipEnrichmentStressDivergenceTensors);

  // Materials
  registerMaterial(ComputeCrackTipEnrichmentSmallStrain);

  // BC's
  registerBoundaryCondition(CrackTipEnrichmentCutOffBC);
}

void
XFEMApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  SolidMechanicsApp::associateSyntax(syntax, action_factory);
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
}

// External entry point for dynamic syntax association
extern "C" void
XFEMApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  XFEMApp::associateSyntax(syntax, action_factory);
}
void
XFEMApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  registerTask("setup_xfem", false);
  registerAction(XFEMAction, "setup_xfem");
  syntax.addDependency("setup_xfem", "setup_adaptivity");
  registerAction(XFEMAction, "add_aux_variable");
  registerAction(XFEMAction, "add_aux_kernel");
  registerAction(XFEMAction, "add_variable");
  registerAction(XFEMAction, "add_kernel");
  registerAction(XFEMAction, "add_bc");

  registerSyntax("XFEMAction", "XFEM");
}

void
XFEMApp::registerExecFlags()
{
  MooseApp::registerExecFlags();
  registerExecFlag(EXEC_XFEM_MARK);
}
