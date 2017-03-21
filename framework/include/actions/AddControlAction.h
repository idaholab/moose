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

#ifndef ADDCONTROLACTION_H
#define ADDCONTROLACTION_H

// MOOSE includes
#include "MooseObjectAction.h"

// Forward declarations
class AddControlAction;

template <>
InputParameters validParams<AddControlAction>();

/**
 * Action for creating Control objects
 *
 * Control objects are GeneralUserObjects, thus just
 * use the AddUserObjectAction
 */
class AddControlAction : public MooseObjectAction
{
public:
  /**
   * Class constructor
   * @param params Parameters for this Action
   */
  AddControlAction(InputParameters parameters);

  virtual void act() override;
};

#endif // ADDCONTROLACTION_H
