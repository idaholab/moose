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

#ifndef ADDARRAYKERNELACTION_H
#define ADDARRAYKERNELACTION_H

#include "MooseObjectAction.h"

class AddArrayKernelAction;

template<>
InputParameters validParams<AddArrayKernelAction>();


class AddArrayKernelAction : public MooseObjectAction
{
public:
  AddArrayKernelAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDARRAYKERNELACTION_H
