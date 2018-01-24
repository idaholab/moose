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

#ifndef CREATEDISPLACEDPROBLEMACTION_H
#define CREATEDISPLACEDPROBLEMACTION_H

#include "Action.h"

class CreateDisplacedProblemAction;

template <>
InputParameters validParams<CreateDisplacedProblemAction>();

/**
 *
 */
class CreateDisplacedProblemAction : public Action
{
public:
  CreateDisplacedProblemAction(InputParameters parameters);

  virtual void act() override;
};

#endif /* CREATEDISPLACEDPROBLEMACTION_H */
