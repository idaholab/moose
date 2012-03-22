#include "ContactModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

// contact
#include "ContactAction.h"
#include "ContactMaster.h"
#include "SlaveConstraint.h"
#include "OneDContactConstraint.h"
#include "MultiDContactConstraint.h"

void
Elk::Contact::registerObjects()
{
  // contact
  registerAction(ContactAction, "meta_action");
  registerDiracKernel(ContactMaster);
  registerDiracKernel(SlaveConstraint);
  registerConstraint(OneDContactConstraint);
  registerConstraint(MultiDContactConstraint);
}

void
Elk::Contact::associateSyntax()
{
  Moose::syntax.registerActionSyntax("ContactAction", "Contact/*");
  Moose::syntax.registerActionSyntax("ThermalContactAction", "ThermalContact/*");
}
