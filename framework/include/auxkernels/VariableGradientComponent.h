//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VARIABLEGRADIENTCOMPONENT_H
#define VARIABLEGRADIENTCOMPONENT_H

// MOOSE includes
#include "AuxKernel.h"

// Forward declarations
class VariableGradientComponent;

template <>
InputParameters validParams<VariableGradientComponent>();

/**
 * Extract a component from the gradient of a variable
 */
class VariableGradientComponent : public AuxKernel
{
public:
  /**
   * Class constructor
   * @param parameters Input parameters for the object
   */
  VariableGradientComponent(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Reference to the gradient of the coupled variable
  const VariableGradient & _gradient;

  /// Desired component
  int _component;
};

#endif // VARIABLEGRADIENTCOMPONENT_H
