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
 * Calculates the order parameter from the linearized interface function
 */
class LinearizedInterfaceAux : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters Input parameters for the object
   */
  LinearizedInterfaceAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Reference to the variable used in the linearized interface function
  const VariableValue & _phi;
};
