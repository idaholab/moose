/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef HHPFCRFFSPLITKERNELACTION_H
#define HHPFCRFFSPLITKERNELACTION_H

#include "Action.h"

// Forward Declarations
class HHPFCRFFSplitKernelAction;

template <>
InputParameters validParams<HHPFCRFFSplitKernelAction>();

class HHPFCRFFSplitKernelAction : public Action
{
public:
  HHPFCRFFSplitKernelAction(const InputParameters & params);

  virtual void act();

protected:
  const unsigned int _num_L;
  const std::string _L_name_base;
  const VariableName _n_name;
};

#endif // HHPFCRFFSPLITKERNELACTION_H
