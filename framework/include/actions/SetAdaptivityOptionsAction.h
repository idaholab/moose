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

#ifndef SETADAPTIVITYOPTIONSACTION_H
#define SETADAPTIVITYOPTIONSACTION_H

#include "Action.h"

class SetAdaptivityOptionsAction;

template<>
InputParameters validParams<SetAdaptivityOptionsAction>();


class SetAdaptivityOptionsAction : public Action
{
public:
  SetAdaptivityOptionsAction(const std::string & name, InputParameters params);

  virtual void act();
};

#endif // SETADAPTIVITYOPTIONSACTION_H
