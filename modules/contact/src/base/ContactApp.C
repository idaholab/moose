//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "ContactAction.h"
#include "ContactMaster.h"
#include "ContactPenetrationAuxAction.h"
#include "ContactPenetrationVarAction.h"
#include "ContactPressureAux.h"
#include "ContactPressureAuxAction.h"
#include "ContactPressureVarAction.h"
#include "NodalAreaAction.h"
#include "SlaveConstraint.h"
#include "OneDContactConstraint.h"
#include "MultiDContactConstraint.h"
#include "GluedContactConstraint.h"
#include "MechanicalContactConstraint.h"
#include "SparsityBasedContactConstraint.h"
#include "AugmentedLagrangianContactProblem.h"
#include "ReferenceResidualProblem.h"
#include "NodalArea.h"
#include "NodalAreaAction.h"
#include "NodalAreaVarAction.h"
#include "ContactSlipDamper.h"
#include "ContactSplit.h"

template <>
InputParameters
validParams<ContactApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ContactApp::ContactApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ContactApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ContactApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  ContactApp::registerExecFlags(_factory);
}

ContactApp::~ContactApp() {}

// External entry point for dynamic application loading
extern "C" void
ContactApp__registerApps()
{
  ContactApp::registerApps();
}
void
ContactApp::registerApps()
{
  registerApp(ContactApp);
}

// External entry point for dynamic object registration
extern "C" void
ContactApp__registerObjects(Factory & factory)
{
  ContactApp::registerObjects(factory);
}
void
ContactApp::registerObjects(Factory & factory)
{
  registerDiracKernel(ContactMaster);
  registerDiracKernel(SlaveConstraint);
  registerConstraint(OneDContactConstraint);
  registerConstraint(MultiDContactConstraint);
  registerConstraint(GluedContactConstraint);
  registerConstraint(MechanicalContactConstraint);
  registerConstraint(SparsityBasedContactConstraint);
  registerProblem(AugmentedLagrangianContactProblem);
  registerProblem(ReferenceResidualProblem);
  registerUserObject(NodalArea);
  registerAux(ContactPressureAux);
  registerDamper(ContactSlipDamper);
  registerSplit(ContactSplit);
}

// External entry point for dynamic syntax association
extern "C" void
ContactApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  ContactApp::associateSyntax(syntax, action_factory);
}
void
ContactApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  registerSyntax("ContactAction", "Contact/*");

  registerSyntax("ContactPenetrationAuxAction", "Contact/*");
  registerSyntax("ContactPenetrationVarAction", "Contact/*");

  registerSyntax("ContactPressureAuxAction", "Contact/*");
  registerSyntax("ContactPressureVarAction", "Contact/*");

  registerSyntax("NodalAreaAction", "Contact/*");
  registerSyntax("NodalAreaVarAction", "Contact/*");

  registerAction(ContactAction, "add_aux_kernel");
  registerAction(ContactAction, "add_aux_variable");
  registerAction(ContactAction, "add_dirac_kernel");

  registerTask("output_penetration_info_vars", false);
  registerAction(ContactAction, "output_penetration_info_vars");
  syntax.addDependency("output_penetration_info_vars", "add_output");

  registerAction(ContactPenetrationAuxAction, "add_aux_kernel");
  registerAction(ContactPenetrationVarAction, "add_aux_variable");

  registerAction(ContactPressureAuxAction, "add_aux_kernel");
  registerAction(ContactPressureVarAction, "add_aux_variable");

  registerAction(NodalAreaAction, "add_user_object");
  registerAction(NodalAreaVarAction, "add_aux_variable");
}

// External entry point for dynamic execute flag registration
extern "C" void
ContactApp__registerExecFlags(Factory & factory)
{
  ContactApp::registerExecFlags(factory);
}
void
ContactApp::registerExecFlags(Factory & /*factory*/)
{
}
