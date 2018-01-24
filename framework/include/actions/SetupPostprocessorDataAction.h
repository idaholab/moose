//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SETUPPOSTPROCESSORDATAACTION_H
#define SETUPPOSTPROCESSORDATAACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class SetupPostprocessorDataAction;

template <>
InputParameters validParams<SetupPostprocessorDataAction>();

/**
 * The PostprocessorInterface::hasPostprocessor method utilizes the PostprocessorData
 * to determine if a postprocessor exists. Since many objects (e.g., Functions) are created
 * prior to the creation of Postprocessors, the PostprocessorData must be populated much eariler
 * than the actual creation of the Postprocessors if the correct behavior of hasPostprocessor is
 * to be achieved. Hence, this action simply initializes the PostprocessorData.
 */
class SetupPostprocessorDataAction : public Action
{
public:
  /**
   * Class constructor
   * @param params Input parameters for the action
   */
  SetupPostprocessorDataAction(InputParameters params);

  virtual void act() override;
};

#endif // SETUPPOSTPROCESSORDATAACTION_H
