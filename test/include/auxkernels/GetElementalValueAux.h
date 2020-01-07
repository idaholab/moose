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
 * Tests the \c getElementalValue() function of \c MooseVariableFE.
 */
class GetElementalValueAux : public AuxKernel
{
public:
  static InputParameters validParams();

  GetElementalValueAux(const InputParameters & params);

protected:
  virtual Real computeValue() override;

  /// Variable to be copied
  const MooseVariable & _copied_var;

  /// Time level of the copied variable
  const MooseEnum & _time_level;
};
