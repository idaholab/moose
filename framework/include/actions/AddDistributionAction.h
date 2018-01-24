//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDDISTRIBUTIONACTION_H
#define ADDDISTRIBUTIONACTION_H

#include "MooseObjectAction.h"

class AddDistributionAction;

template <>
InputParameters validParams<AddDistributionAction>();

/**
 * This class adds a distribution object.
 */
class AddDistributionAction : public MooseObjectAction
{
public:
  AddDistributionAction(InputParameters params);

  virtual void act() override;
};

#endif /* ADDDISTRIBUTIONACTION_H */
