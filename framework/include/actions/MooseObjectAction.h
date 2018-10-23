//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEOBJECTACTION_H
#define MOOSEOBJECTACTION_H

#include "MooseObjectActionBase.h"

#include <string>

class MooseObjectAction;

template <>
InputParameters validParams<MooseObjectAction>();

class MooseObjectAction : public MooseObjectActionBase
{
public:
  MooseObjectAction(InputParameters params);
};

#endif // MOOSEOBJECTACTION_H
