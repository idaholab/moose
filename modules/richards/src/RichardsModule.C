#include "RichardsModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

#include "RichardsDensityConstBulk.h"
#include "RichardsDensityIdeal.h"
#include "RichardsRelPermPower.h"
#include "RichardsRelPermVG.h"
#include "RichardsRelPermVG1.h"
#include "RichardsSeffVG.h"
#include "RichardsSeffVG1.h"

void
Elk::Richards::registerObjects(Factory & factory)
{
  registerUserObject(RichardsDensityConstBulk);
  registerUserObject(RichardsDensityIdeal);
  registerUserObject(RichardsRelPermPower);
  registerUserObject(RichardsRelPermVG);
  registerUserObject(RichardsRelPermVG1);
  registerUserObject(RichardsSeffVG);
  registerUserObject(RichardsSeffVG1);
}

void
Elk::Richards::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
}
