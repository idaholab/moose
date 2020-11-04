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
class VectorVariableMagnitudeAux : public AuxKernel {
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters Input parameters for the object
   */
  VectorVariableMagnitudeAux(const InputParameters &parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Pointer to elemental variable value
  const VectorVariableValue &_variable_value;
};
