//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableFV.h"
#include "MooseLinearVariableFV.h"
#include "FVUtils.h"
#include "SystemBase.h"

namespace Moose
{
namespace FV
{

bool
elemHasFaceInfo(const Elem & elem, const Elem * const neighbor)
{
  // The face info belongs to elem:
  //  * at all mesh boundaries (i.e. where there is no neighbor)
  //  * if the element faces a neighbor which is on a lower refinement level
  //  * if the element is active and it has a lower ID than its neighbor
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
