//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BADADDKERNELACTION_H
#define BADADDKERNELACTION_H

#include "MooseObjectAction.h"

class BadAddKernelAction;

template <>
InputParameters validParams<BadAddKernelAction>();

/**
 * This class is for testing an error condition. It is registered
 * to add kernels but is calling the wrong method on FEProblemBase
 * to actually add the object. DO NOT COPY!
 */
class BadAddKernelAction : public MooseObjectAction
{
public:
  BadAddKernelAction(InputParameters params);

  virtual void act();
};

#endif // BADADDKERNELACTION_H
