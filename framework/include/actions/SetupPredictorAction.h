//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SETUPPREDICTORACTION_H
#define SETUPPREDICTORACTION_H

#include "MooseObjectAction.h"

class SetupPredictorAction;

template <>
InputParameters validParams<SetupPredictorAction>();

/**
 * Sets the predictor
 */
class SetupPredictorAction : public MooseObjectAction
{
public:
  SetupPredictorAction(InputParameters parameters);

  virtual void act() override;
};

#endif /* SETUPPREDICTORACTION_H */
