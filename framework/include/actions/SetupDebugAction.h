//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SETUPDEBUGACTION_H
#define SETUPDEBUGACTION_H

// MOOSE includes
#include "Action.h"

class SetupDebugAction;
class MooseObjectAction;

template <>
InputParameters validParams<SetupDebugAction>();

/**
 *
 */
class SetupDebugAction : public Action
{
public:
  SetupDebugAction(InputParameters parameters);

  virtual void act() override;

protected:
  /**
   * Helper method for creating Output actions
   * @param type The type of object to create (e.g., TopResidualDebugOutput)
   * @param name The name to give the object being created
   * @return A pointer to the OutputAction that was generated
   */
  MooseObjectAction * createOutputAction(const std::string & type, const std::string & name);

  /// Parameters from the action being created (AddOutputAction)
  InputParameters _action_params;
};

#endif /* SETUPDEBUGACTION_H */
