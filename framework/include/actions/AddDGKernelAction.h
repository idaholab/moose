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

#ifndef ADDDGKERNELACTION_H
#define ADDDGKERNELACTION_H

#include "MooseObjectAction.h"

class AddDGKernelAction;

template <>
InputParameters validParams<AddDGKernelAction>();

class AddDGKernelAction : public MooseObjectAction
{
public:
  AddDGKernelAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDKERNELACTION_H
