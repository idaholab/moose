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
class MultipleUpdateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  MultipleUpdateAux(const InputParameters & parameters);
  virtual ~MultipleUpdateAux();

protected:
  virtual Real computeValue();

  const VariableValue & _nl_u;

  /// use deprecated API
  const bool _deprecated;

  /// current API
  MooseVariable * _var1;
  MooseVariable * _var2;

  /// deprectated API
  VariableValue * _dvar1;
  VariableValue * _dvar2;
};
