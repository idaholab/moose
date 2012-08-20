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

#ifndef SETUPDAMPERSACTION_H
#define SETUPDAMPERSACTION_H

#include "Action.h"

class SetupDampersAction;

template<>
InputParameters validParams<SetupDampersAction>();


class SetupDampersAction : public Action
{
public:
  SetupDampersAction(const std::string & name, InputParameters params);

  virtual void act();
};

#endif // SETUPDAMPERSACTION_H
