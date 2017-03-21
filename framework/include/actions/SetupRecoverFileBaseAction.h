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

#ifndef SETUPRECOVERFILEBASEACTION_H
#define SETUPRECOVERFILEBASEACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class SetupRecoverFileBaseAction;

template <>
InputParameters validParams<SetupRecoverFileBaseAction>();

/**
 *
 */
class SetupRecoverFileBaseAction : public Action
{
public:
  /**
   * Class constructor
   * @param params Input parameters for this action
   */
  SetupRecoverFileBaseAction(InputParameters params);

  virtual void act() override;
};

#endif // SETUPRECOVERFILEBASEACTION_H
