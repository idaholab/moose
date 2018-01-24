//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PFCRFFKERNELACTION_H
#define PFCRFFKERNELACTION_H

#include "HHPFCRFFSplitKernelAction.h"

// Forward Declarations
class PFCRFFKernelAction;

template <>
InputParameters validParams<PFCRFFKernelAction>();

class PFCRFFKernelAction : public HHPFCRFFSplitKernelAction
{
public:
  PFCRFFKernelAction(const InputParameters & params);

  virtual void act();
};

#endif // PFCRFFKERNELACTION_H
