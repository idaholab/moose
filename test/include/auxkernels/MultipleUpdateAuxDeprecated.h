//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Aux kernel that updated values of coupled variables
 */
class MultipleUpdateAuxDeprecated : public AuxKernel
{
public:
  static InputParameters validParams();

  MultipleUpdateAuxDeprecated(const InputParameters & parameters);
  virtual ~MultipleUpdateAuxDeprecated();

protected:
  virtual Real computeValue();

  const VariableValue & _nl_u;
  VariableValue & _var1;
  VariableValue & _var2;
};
