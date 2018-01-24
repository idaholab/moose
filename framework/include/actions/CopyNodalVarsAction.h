//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COPYNODALVARSACTION_H
#define COPYNODALVARSACTION_H

#include "Action.h"

class CopyNodalVarsAction;

template <>
InputParameters validParams<CopyNodalVarsAction>();

class CopyNodalVarsAction : public Action
{
public:
  CopyNodalVarsAction(InputParameters params);

  virtual void act() override;
};

#endif // COPYNODALVARSACTION_H
