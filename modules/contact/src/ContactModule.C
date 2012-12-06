#include "ContactModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "ProblemFactory.h"
#include "Parser.h"

// contact
#include "ContactAction.h"
#include "ContactMaster.h"
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
}

void
Elk::Contact::associateSyntax(Syntax & syntax)
{
  syntax.registerActionSyntax("ContactAction", "Contact/*");

  registerAction(ContactAction, "add_dg_kernel");
}
