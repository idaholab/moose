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

#ifndef ADDINTERFACEKERNELACTION_H
#define ADDINTERFACEKERNELACTION_H

#include "MooseObjectAction.h"

class AddInterfaceKernelAction;

template <>
InputParameters validParams<AddInterfaceKernelAction>();

class AddInterfaceKernelAction : public MooseObjectAction
{
public:
  AddInterfaceKernelAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDKERNELACTION_H
