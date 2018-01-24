/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RigidBodyMultiKernelAction_H
#define RigidBodyMultiKernelAction_H

#include "Action.h"

class RigidBodyMultiKernelAction : public Action
{
public:
  RigidBodyMultiKernelAction(const InputParameters & params);

  virtual void act();
  unsigned int _op_num;
  std::string _var_name_base;
  bool _implicit;
};

template <>
InputParameters validParams<RigidBodyMultiKernelAction>();

#endif // RigidBodyMultiKernelAction_H
