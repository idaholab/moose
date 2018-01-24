//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONVECTIONDIFFUSIONACTION_H
#define CONVECTIONDIFFUSIONACTION_H

#include "Action.h"

class ConvectionDiffusionAction : public Action
{
public:
  ConvectionDiffusionAction(InputParameters params);

  virtual void act() override;
};

template <>
InputParameters validParams<ConvectionDiffusionAction>();

#endif // CONVECTIONDIFFUSIONACTION_H
