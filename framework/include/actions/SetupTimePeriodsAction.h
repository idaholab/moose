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

#ifndef SETUPTIMEPERIODSACTION_H
#define SETUPTIMEPERIODSACTION_H

#include "MooseObjectAction.h"

class SetupTimePeriodsAction;

template<>
InputParameters validParams<SetupTimePeriodsAction>();


class SetupTimePeriodsAction : public Action
{
public:
  SetupTimePeriodsAction(const std::string & name, InputParameters params);

  virtual void act();
};

#endif // SETUPTIMEPERIODSACTION_H
