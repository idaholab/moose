/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ADDSLAVEFLUXVECTORACTION_H
#define ADDSLAVEFLUXVECTORACTION_H

#include "MooseObjectAction.h"

#include <string>

class AddSlaveFluxVectorAction : public Action
{
public:
  AddSlaveFluxVectorAction(const InputParameters & params);

  virtual void act();
};

template <>
InputParameters validParams<AddSlaveFluxVectorAction>();

#endif // ADDSLAVEFLUXVECTORACTION_H
