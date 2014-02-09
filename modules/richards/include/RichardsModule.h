/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSMODULE_H
#define RICHARDSMODULE_H

#include "Factory.h"
#include "Syntax.h"
#include "ActionFactory.h"

namespace Elk
{
  namespace Richards
  {
    void registerObjects(Factory & factory);
    void associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/);
  }
}

#endif //RICHARDSMODULE_H
