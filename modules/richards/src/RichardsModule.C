#include "RichardsModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

#include "RichardsDensityConstBulk.h"
#include "RichardsDensityIdeal.h"

void
Elk::Richards::registerObjects(Factory & factory)
{
  registerUserObject(RichardsDensityConstBulk);
  registerUserObject(RichardsDensityIdeal);
}

void
Elk::Richards::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
}
