//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSESYNTAX_H
#define MOOSESYNTAX_H

class Syntax;
class ActionFactory;

namespace Moose
{
void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
}

#endif // MOOSESYNTAX_H
