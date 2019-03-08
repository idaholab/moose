//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VECTORVARIABLECOMPONENTAUX_H
#define VECTORVARIABLECOMPONENTAUX_H

// MOOSE includes
#include "AuxKernel.h"

// Forward declarations
class VectorVariableComponentAux;

template <>
InputParameters validParams<VectorVariableComponentAux>();

/**
 * Extract a component from a vector variable
 */
class VectorVariableComponentAux : public AuxKernel
{
public:
  /**
   * Class constructor
   * @param parameters Input parameters for the object
   */
  VectorVariableComponentAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Reference to the value of the coupled vector variable
  const RealVectorValue & _vector_variable_value;

  /// Desired component
  int _component;
};

#endif // VECTORVARIABLECOMPONENTAUX_H
