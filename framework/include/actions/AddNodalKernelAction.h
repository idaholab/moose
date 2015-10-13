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

#ifndef ADDNODALKERNELACTION_H
#define ADDNODALKERNELACTION_H

#include "MooseObjectAction.h"

class AddNodalKernelAction;

template<>
InputParameters validParams<AddNodalKernelAction>();


class AddNodalKernelAction : public MooseObjectAction
{
public:
  AddNodalKernelAction(InputParameters params);

  virtual void act();
};

#endif // ADDNODALKERNELACTION_H
