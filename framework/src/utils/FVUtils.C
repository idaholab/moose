//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableFV.h"
#include "FVUtils.h"

namespace Moose
{
namespace FV
{

bool
elemHasFaceInfo(const Elem & elem, const Elem * const neighbor)
{
  if (!neighbor)
    return true;
  else if (elem.level() != neighbor->level())
    return neighbor->level() < elem.level();
  else if (!neighbor->active())
    return false;
  else
    return elem.id() < neighbor->id();
}

}
}
