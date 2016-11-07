/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
