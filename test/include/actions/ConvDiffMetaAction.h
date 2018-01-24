//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONVDIFFMETAACTION_H
#define CONVDIFFMETAACTION_H

#include "Action.h"

class ConvDiffMetaAction : public Action
{
public:
  ConvDiffMetaAction(const InputParameters & params);

  virtual void act();
};

template <>
InputParameters validParams<ConvDiffMetaAction>();

#endif // CONVDIFFMETAACTION_H
