/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CHPFCRFFSPLITKERNELACTION_H
#define CHPFCRFFSPLITKERNELACTION_H

#include "Action.h"

// Forward Declarations
class CHPFCRFFSplitKernelAction;

template <>
InputParameters validParams<CHPFCRFFSplitKernelAction>();

/**
 * \todo Needs documentation.
 */
class CHPFCRFFSplitKernelAction : public Action
{
public:
  CHPFCRFFSplitKernelAction(const InputParameters & params);

  virtual void act();

private:
  const unsigned int _num_L;
  const std::string _L_name_base;
  const NonlinearVariableName _n_name;
};

#endif // CHPFCRFFSPLITKERNELACTION_H
