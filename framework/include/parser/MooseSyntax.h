/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MOOSESYNTAX_H
#define MOOSESYNTAX_H

#include <string>
#include <map>

class Syntax;
class ActionFactory;

namespace Moose
{
void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
}

#endif // MOOSESYNTAX_H
