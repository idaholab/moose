//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseBaseErrorInterface.h"
#include "MooseBase.h"
#include "MooseApp.h"

#include "hit/parse.h"

MooseBaseErrorInterface::MooseBaseErrorInterface(const MooseBase & base)
  : ConsoleStreamInterface(base.getMooseApp()), _moose_base(base)
{
}
