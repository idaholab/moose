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

#ifndef SETUPSINGLEMATRIXACTION_H
#define SETUPSINGLEMATRIXACTION_H

#include "Action.h"

class SetupSingleMatrixAction;

template<>
InputParameters validParams<SetupSingleMatrixAction>();

/**
 * Action to setup single matrix Jacobian (or Jacobian approximate)
 *
 */
class SetupSingleMatrixAction: public Action
{
public:
  SetupSingleMatrixAction(const std::string & name, InputParameters params);

  virtual void act();
};

#endif //SETUPSINGLEMATRIXACTION_H
