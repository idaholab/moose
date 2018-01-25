//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMApp.h"
#include "SolidMechanicsApp.h"
#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "XFEMVolFracAux.h"
#include "XFEMCutPlaneAux.h"
#include "XFEMMarkerAux.h"
#include "XFEMMarkerUserObject.h"
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

  Moose::registerExecFlags(_factory);
  XFEMApp::registerExecFlags(_factory);
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
  registerUserObject(XFEMMarkerUserObject);
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

// External entry point for dynamic execute flag registration
extern "C" void
XFEMApp__registerExecFlags(Factory & factory)
{
  XFEMApp::registerExecFlags(factory);
}
void
XFEMApp::registerExecFlags(Factory & /*factory*/)
{
}
