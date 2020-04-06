//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralPostprocessor.h"

// Forward Declarations
class Function;
class MooseVariableScalar;

/**
 * Postprocessor for computing the error in a scalar value relative to
 * a known Function's value.
 */
class ScalarL2Error : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ScalarL2Error(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  /**
   * Get the L2 Error.
   */
  virtual Real getValue() override;

protected:
  MooseVariableScalar & _var;
  const Function & _func;
};
