/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALKERNELACTION_H
#define POLYCRYSTALKERNELACTION_H

#include "Action.h"

class PolycrystalKernelAction: public Action
{
public:
  PolycrystalKernelAction(const InputParameters & params);

  virtual void act();

private:
  unsigned int _op_num;
  std::string _var_name_base;
  bool _implicit;
};

template<>
InputParameters validParams<PolycrystalKernelAction>();

#endif //POLYCRYSTALKERNELACTION_H
