//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Action.h"

/**
 * Action that create Application Block. It is noted that most information has already been
 * parsed before the MOOSE object is built. This action mainly serve the purpose of completing
 * the action system workflow for MOOSE objects.
 */
class CreateApplicationBlockAction : public Action
{
public:
  static InputParameters validParams();

  CreateApplicationBlockAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  // The string to store application type
  const std::string _type;
};
