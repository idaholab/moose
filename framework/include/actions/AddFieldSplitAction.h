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

#ifndef ADDFIELDSPLITACTION_H
#define ADDFIELDSPLITACTION_H

#include "Action.h"

class AddFieldSplitAction;

template<>
InputParameters validParams<AddFieldSplitAction>();


class AddFieldSplitAction : public Action
{
public:
  AddFieldSplitAction(const std::string & name, InputParameters params);
  virtual void act();
};

#endif // ADDFIELDSPLITACTION_H
