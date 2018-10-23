//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEADOBJECTACTION_H
#define MOOSEADOBJECTACTION_H

#include "MooseObjectActionBase.h"

#include <string>

class MooseADObjectAction;

template <>
InputParameters validParams<MooseADObjectAction>();

class MooseADObjectAction : public MooseObjectActionBase
{
public:
  MooseADObjectAction(InputParameters params);
};

#endif // MOOSEADOBJECTACTION_H
