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
#include "AuxKernel.h"

/**
 * Extract a component from a vector variable
 */
class VectorVariableComponentAux : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters Input parameters for the object
   */
  VectorVariableComponentAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Pointer to nodal variable value
  const RealVectorValue * const _nodal_variable_value;

  /// Pointer to elemental variable value
  const VectorVariableValue * const _elemental_variable_value;

  /// Desired component
  const int _component;
};
