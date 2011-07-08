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

#ifndef SETUPSMPACTION_H
#define SETUPSMPACTION_H

#include "SetupPreconditionerAction.h"

class SetupSMPAction;

template<>
InputParameters validParams<SetupSMPAction>();

/**
 * Action to setup single matrix Jacobian (or Jacobian approximate)
 *
 */
class SetupSMPAction: public SetupPreconditionerAction
{
public:
  SetupSMPAction(const std::string & name, InputParameters params);

  virtual void act();
};

#endif //SETUPSINGLEMATRIXACTION_H
