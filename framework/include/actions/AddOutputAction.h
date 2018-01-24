//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDOUTPUTACTION_H
#define ADDOUTPUTACTION_H

// MOOSE includes
#include "MooseObjectAction.h"

// Forward declerations
class AddOutputAction;

template <>
InputParameters validParams<AddOutputAction>();

/**
 * Action for creating output objects
 */
class AddOutputAction : public MooseObjectAction
{
public:
  /**
   * Class constructor
   */
  AddOutputAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDOUTPUTACTION_H
