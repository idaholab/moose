#include "ContactModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

// contact
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
#include "FrictionalContactProblem.h"
#include "ReferenceResidualProblem.h"

void
Elk::Contact::registerObjects()
{
  // contact
  registerDiracKernel(ContactMaster);
  registerDiracKernel(SlaveConstraint);
  registerConstraint(OneDContactConstraint);
  registerConstraint(MultiDContactConstraint);
  registerProblem(FrictionalContactProblem);
  registerProblem(ReferenceResidualProblem);

  registerAux(ContactPressureAux);
}

void
Elk::Contact::associateSyntax(Syntax & syntax)
{
  syntax.registerActionSyntax("ContactAction", "Contact/*");

  syntax.registerActionSyntax("ContactPenetrationAuxAction", "Contact/*");
  syntax.registerActionSyntax("ContactPenetrationVarAction", "Contact/*");

  syntax.registerActionSyntax("ContactPressureAuxAction", "Contact/*");
  syntax.registerActionSyntax("ContactPressureVarAction", "Contact/*");

  registerAction(ContactAction, "add_dg_kernel");

  registerAction(ContactPenetrationAuxAction, "add_aux_bc");
  registerAction(ContactPenetrationVarAction, "add_aux_variable");

  registerAction(ContactPressureAuxAction, "add_aux_bc");
  registerAction(ContactPressureVarAction, "add_aux_variable");
}
