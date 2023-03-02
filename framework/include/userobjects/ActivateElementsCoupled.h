//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActivateElementsUserObjectBase.h"
#include "MooseEnum.h"

class ActivateElementsCoupled : public ActivateElementsUserObjectBase
{
public:
  static InputParameters validParams();

  ActivateElementsCoupled(const InputParameters & parameters);

  virtual bool isElementActivated() override;

protected:
  /// variable value to decide wether an element whould be activated
  const VariableValue & _coupled_var;
  /// variable value to decide wether an element whould be activated
  const Real _activate_value;
  /// type of activation - blow or above
  const enum class ActivateType { BELOW, EQUAL, ABOVE } _activate_type;
};
