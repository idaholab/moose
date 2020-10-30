//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "Action.h"
class SetupPostprocessorDataAction;

template <>
InputParameters validParams<SetupPostprocessorDataAction>();

/**
 * The PostprocessorInterface::getPostprocessorValue methods rely on
 * FEProblemBase::hasPostprocessorByName to determine if the default (if supplied) should be used.
 * As such all the PostprocessorValues must be declared prior to any calls to the get methods
 * Hence, this action simply initializes the Postprocessor data prior to object construction.
 */
class SetupPostprocessorDataAction : public Action
{
public:
  static InputParameters validParams();
  SetupPostprocessorDataAction(InputParameters params);
  virtual void act() override;
};
