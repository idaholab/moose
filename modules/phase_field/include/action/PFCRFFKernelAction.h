/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
