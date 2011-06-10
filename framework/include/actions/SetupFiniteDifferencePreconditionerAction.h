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

#ifndef SETUPFINITEDIFFERENCEPRECONDITIONERACTION_H
#define SETUPFINITEDIFFERENCEPRECONDITIONERACTION_H

#include "Action.h"

class SetupFiniteDifferencePreconditionerAction;

template<>
InputParameters validParams<SetupFiniteDifferencePreconditionerAction>();

/**
 * Action to setup single matrix Jacobian (or Jacobian approximate)
 *
 */
class SetupFiniteDifferencePreconditionerAction: public Action
{
public:
  SetupFiniteDifferencePreconditionerAction(const std::string & name, InputParameters params);

  virtual void act();
};

#endif //SETUPFINITEDIFFERENCEPRECONDITIONERACTION_H
