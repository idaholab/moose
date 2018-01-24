//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
