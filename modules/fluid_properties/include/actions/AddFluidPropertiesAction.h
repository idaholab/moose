/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef ADDFLUIDPROPERTIESACTION_H
#define ADDFLUIDPROPERTIESACTION_H

#include "AddUserObjectAction.h"

class AddFluidPropertiesAction;

template <>
InputParameters validParams<AddFluidPropertiesAction>();

class AddFluidPropertiesAction : public AddUserObjectAction
{
public:
  AddFluidPropertiesAction(InputParameters params);
};

#endif /* ADDFLUIDPROPERTIESACTION_H */
