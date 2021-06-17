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
 * Compute the magnitude of a vector variable
 */
class VectorVariableMagnitudeAux : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters Input parameters for the object
   */
  VectorVariableMagnitudeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Reference to vector variable value
  const VectorVariableValue & _variable_value;
};
