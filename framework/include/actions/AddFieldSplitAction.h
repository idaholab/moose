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

#include "MooseObjectAction.h"

class AddFieldSplitAction;

template <>
InputParameters validParams<AddFieldSplitAction>();

class AddFieldSplitAction : public MooseObjectAction
{
public:
  // constructor
  AddFieldSplitAction(InputParameters params);
  // prepare PETSc options
  void act();
};

#endif /* ADDFIELDSPLITACTION_H */
