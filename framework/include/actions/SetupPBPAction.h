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

#ifndef SETUPPBPACTION_H
#define SETUPPBPACTION_H

#include "Action.h"

class SetupPBPAction: public Action
{
public:
  SetupPBPAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<SetupPBPAction>();

#endif //SETUPPBPACTION_H
