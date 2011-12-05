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

#ifndef INITDISPLACEDPROBLEMACTION_H
#define INITDISPLACEDPROBLEMACTION_H

#include "Action.h"

class InitDisplacedProblemAction;

template<>
InputParameters validParams<InitDisplacedProblemAction>();

/**
 *
 */
class InitDisplacedProblemAction : public Action
{
public:
  InitDisplacedProblemAction(const std::string & name, InputParameters parameters);
  virtual ~InitDisplacedProblemAction();

  virtual void act();

protected:

};

#endif /* INITDISPLACEDPROBLEMACTION_H */
